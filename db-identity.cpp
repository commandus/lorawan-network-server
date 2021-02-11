#include <sstream>
#include "db-identity.h"
#include "iostream"
#include "errlist.h"
#include <string.h>

#include <sys/ipc.h> 
#include <sys/msg.h> 

#define LOG(verbosity) std::cerr

dbenv::dbenv(
	const std::string &apath,
	int aflags,
	int amode
)
	: path(apath), flags(aflags), mode(amode)
{ }

static int processMapFull
(
	dbenv *env
)
{
	mdb_txn_abort(env->txn);
	struct MDB_envinfo current_info;
	int r;
#ifdef USE_LMDB	
	r = mdb_env_info(env->env, &current_info);
#else
	r = mdb_env_info(env->env, &current_info, sizeof(current_info));
#endif	
	if (r)
	{
		LOG(ERROR) << "map full, mdb_env_info error " << r << ": " << strerror_client(r) << std::endl;
		return r;
	}
	if (!closeDb(env))
	{
		LOG(ERROR) << "map full, error close database " << std::endl;
		return ERR_CODE_LMDB_CLOSE;
	}
	size_t new_size = current_info.me_mapsize * 2;
	LOG(INFO) << "map full, doubling map size from " << current_info.me_mapsize << " to " << new_size << " bytes" << std::endl;

	r = mdb_env_create(&env->env);
	if (r)
	{
		LOG(ERROR) << "map full, mdb_env_create error " << r << ": " << strerror_client(r) << std::endl;
		env->env = NULL;
		return ERR_CODE_LMDB_OPEN;
	}
	r = mdb_env_set_mapsize(env->env, new_size);
	if (r)
		LOG(ERROR) << "map full, mdb_env_set_mapsize error " << r << ": " << strerror_client(r) << std::endl;
	r = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
	mdb_env_close(env->env);
	if (!openDb(env))
	{
		LOG(ERROR) << "map full, error re-open database" << std::endl;
		return r;
	}

	// start transaction
	r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
		LOG(ERROR) << "map full, begin transaction error " << r << ": " << strerror_client(r) << std::endl;
	return r;
}

/**
 * @brief Opens LMDB database file
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb
(
	dbenv *env
)
{
	int rc = mdb_env_create(&env->env);
	if (rc)
	{
		LOG(ERROR) << "mdb_env_create error " << rc << ": " << strerror_client(rc) << std::endl;
		env->env = NULL;
		return false;
	}

	rc = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
	if (rc)
	{
		LOG(ERROR) << "mdb_env_open path: " << env->path << " error " << rc  << ": " << strerror_client(rc) << std::endl;
		env->env = NULL;
		return false;
	}

	rc = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (rc)
	{
		LOG(ERROR) << "mdb_txn_begin error " << rc << ": " << strerror_client(rc) << std::endl;
		env->env = NULL;
		return false;
	}

	rc = mdb_dbi_open(env->txn, NULL, 0, &env->dbi);
	if (rc)
	{
		LOG(ERROR) << "mdb_open error " << rc << ": " << strerror_client(rc) << std::endl;
		env->env = NULL;
		return false;
	}

	rc = mdb_txn_commit(env->txn);

	return rc == 0;
}

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb
(
	dbenv *env
)
{
	mdb_dbi_close(env->env, env->dbi);
	mdb_env_close(env->env);
	return true;
}

#define DBG(n) if (verbosity > 3)	LOG(INFO) << "putAddr " << n << std::endl;

/**
 * @brief Store input packet to the LMDB
 * @param env database env
 * @param addr network address
 * @param deviceid keys
 * @return 0 - success
 */
int putAddr
(
	dbenv *env,
	DEVADDR &addr,
	DEVICEID &deviceid,
	int verbosity
)
{
	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
		return ERR_CODE_LMDB_TXN_BEGIN;
	MDB_val dbkey;
	dbkey.mv_size = sizeof(DEVADDR);
	dbkey.mv_data = &addr;
	MDB_val dbdata;
	dbdata.mv_size = sizeof(DEVICEID);
	dbdata.mv_data = &deviceid;
	r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
	if (r) {
		if (r == MDB_MAP_FULL) {
			r = processMapFull(env);
			if (r == 0)
				r = mdb_put(env->txn, env->dbi, &dbkey, &dbdata, 0);
		}
		if (r) {
			mdb_txn_abort(env->txn);
			LOG(ERROR) << ERR_LMDB_PUT << r << ": " << strerror_client(r) << std::endl;
			return ERR_CODE_LMDB_PUT;
		}
	}

	r = mdb_txn_commit(env->txn);
	if (r) {
		if (r == MDB_MAP_FULL) {
			r = processMapFull(env);
			if (r == 0) {
				r = mdb_txn_commit(env->txn);
			}
		}
		if (r) {
			LOG(ERROR) << ERR_LMDB_TXN_COMMIT << r << ": " << strerror_client(r) << std::endl;
			return ERR_CODE_LMDB_TXN_COMMIT;
		}
	}
	return r;
}

/**
 * @brief Read log data from the LMDB
 * @param env database env
 * @param DEVADDR Networh address
 * @param onRecord callback
 * @param onRecordEnv object passed to callback
 */
int getAddr
(
	dbenv *env,
	const DEVADDR &addr,
	OnRecord onRecord,
	void *onRecordEnv
)
{
	if (!onRecordEnv)
		return ERR_CODE_WRONG_PARAM;

	if (!onRecord) {
		LOG(ERROR) << ERR_WRONG_PARAM << "onLog" << std::endl;
		return ERR_CODE_WRONG_PARAM;
	}

	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r) {
		LOG(ERROR) << ERR_LMDB_TXN_BEGIN << r << ": " << strerror_client(r) << std::endl;
		return ERR_CODE_LMDB_TXN_BEGIN;
	}

	DEVADDR a;
	int sz;
		
	memset(a, 0, sizeof(DEVADDR));
	MDB_val dbkey;
	dbkey.mv_size = sizeof(DEVADDR);
	dbkey.mv_data = &a;
	// Get the last key
	MDB_cursor *cursor;
	MDB_val dbval;
	r = mdb_cursor_open(env->txn, env->dbi, &cursor);
	if (r != MDB_SUCCESS) {
		LOG(ERROR) << ERR_LMDB_OPEN << r << ": " << strerror_client(r) << std::endl;
		mdb_txn_commit(env->txn);
		return r;
	}
	r = mdb_cursor_get(cursor, &dbkey, &dbval, MDB_SET_RANGE);
	if (r != MDB_SUCCESS) {
		LOG(ERROR) << ERR_LMDB_GET << r << ": " << strerror_client(r) << std::endl;
		mdb_txn_commit(env->txn);
		return r;
	}

	do {
		if (dbval.mv_size < sizeof(DEVICEID))
			continue;
		DEVADDR key1;
		memmove(&key1, dbkey.mv_data, sizeof(DEVADDR));
		DEVICEID v;
		memmove(&v, dbval.mv_data, sizeof(DEVICEID));
		if (onRecord(onRecordEnv, &key1, &v))
			break;
	} while (mdb_cursor_get(cursor, &dbkey, &dbval, MDB_NEXT) == MDB_SUCCESS);

	r = mdb_txn_commit(env->txn);
	if (r) {
		LOG(ERROR) << ERR_LMDB_TXN_COMMIT << r << ": " << strerror_client(r) << std::endl;
		return ERR_CODE_LMDB_TXN_COMMIT;
	}
	return r;
}

/**
 * @brief Remove address from the LMDB
 * @param env database env
 * @param DEVADDR addr
 */
int rmAddr
(
	dbenv *env,
	const DEVADDR &addr			// Networh address
)
{
	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r) {
		LOG(ERROR) << ERR_LMDB_TXN_BEGIN << r << ": " << strerror_client(r) << std::endl;
		return ERR_CODE_LMDB_TXN_BEGIN;
	}

	MDB_val dbkey;
	dbkey.mv_size = sizeof(DEVADDR);
	dbkey.mv_data = (void*) &addr;

	// Get the last key
	MDB_cursor *cursor;
	MDB_val dbval;
	r = mdb_cursor_open(env->txn, env->dbi, &cursor);
	if (r != MDB_SUCCESS) {
		LOG(ERROR) << ERR_LMDB_OPEN << r << ": " << strerror_client(r) << std::endl;
		mdb_txn_commit(env->txn);
		return r;
	}
	r = mdb_cursor_get(cursor, &dbkey, &dbval, MDB_SET_RANGE);
	if (r != MDB_SUCCESS) {
		LOG(ERROR) << ERR_LMDB_GET << r << ": " << strerror_client(r) << std::endl;
		mdb_txn_commit(env->txn);
		return r;
	}

	int cnt = 0;
	do {
		if ((dbval.mv_size < sizeof(DEVADDR)) || (dbkey.mv_size < sizeof(DEVICEID)))
			continue;
		DEVADDR key1;
		memmove(&key1, dbkey.mv_data, sizeof(DEVADDR));
		if (memcmp(&key1, &addr, sizeof(DEVADDR)) != 0)
			break;

		mdb_cursor_del(cursor, 0);
		cnt++;
	} while (mdb_cursor_get(cursor, &dbkey, &dbval, MDB_NEXT) == MDB_SUCCESS);

	r = mdb_txn_commit(env->txn);
	if (r) {
		LOG(ERROR) << ERR_LMDB_TXN_COMMIT << r << std::endl;
		return ERR_CODE_LMDB_TXN_COMMIT;
	}
	if (r == 0)
		r = cnt;
	return r;
}

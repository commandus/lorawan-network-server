#include <sstream>
#include <iostream>
#include <string.h>

#include "db-identity.h"
#include "errlist.h"

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
#ifdef ENABLE_LMDB	
	r = mdb_env_info(env->env, &current_info);
#else
	r = mdb_env_info(env->env, &current_info, sizeof(current_info));
#endif	
	if (r)
		return r;
	if (!closeDb(env))
		return ERR_CODE_LMDB_CLOSE;
	size_t new_size = current_info.me_mapsize * 2;
	//  map full, doubling map size from current_info.me_mapsize to new_size

	r = mdb_env_create(&env->env);
	if (r) {
		env->env = NULL;
		return ERR_CODE_LMDB_OPEN;
	}
	r = mdb_env_set_mapsize(env->env, new_size);
	if (r) {
		// map full, mdb_env_set_mapsize error r, nothing to do
	}
	r = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
	mdb_env_close(env->env);
	if (!openDb(env))
	{
		// map full, error re-open database
		return r;
	}

	// start transaction
	r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r) {
		// map full, begin transaction error
	}
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
		// mdb_env_create error
		env->env = NULL;
		return false;
	}

	rc = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
	if (rc)
	{
		// mdb_env_open path
		env->env = NULL;
		return false;
	}

	rc = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (rc)
	{
		// mdb_txn_begin error
		env->env = NULL;
		return false;
	}

	rc = mdb_dbi_open(env->txn, NULL, 0, &env->dbi);
	if (rc)
	{
		// mdb_open error " << rc << ": " << strerror_lorawan_ns(rc) << std::endl;
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
	DEVICEID &deviceid
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
			return ERR_CODE_LMDB_TXN_COMMIT;
		}
	}
	return r;
}

/**
 * @brief Read log data from the LMDB
 * @param env database env
 * @param DEVADDR Networh address
 * @param DEVICEID return found address properties
 */
int getAddr
(
	dbenv *env,
	const DEVADDR &addr,
	DEVICEID &retval
)
{
	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r)
		return ERR_CODE_LMDB_TXN_BEGIN;

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
		mdb_txn_commit(env->txn);
		return r;
	}
	r = mdb_cursor_get(cursor, &dbkey, &dbval, MDB_SET_RANGE);
	if (r != MDB_SUCCESS) {
		mdb_txn_commit(env->txn);
		return r;
	}

	if (dbval.mv_size < sizeof(DEVICEID))
		return ERR_CODE_INSUFFICIENT_MEMORY;
	memmove(&retval, dbval.mv_data, sizeof(DEVICEID));

	r = mdb_txn_commit(env->txn);
	if (r) {
		return ERR_CODE_LMDB_TXN_COMMIT;
	}
	return r;
}

/**
 * @brief List address
 * @param env database env
 * @param onLog callback
 * @param onLogEnv object passed to callback
 */
int lsAddr
(
	dbenv *env,
	OnRecord onRecord,
	void *onRecordEnv
) {
	// start transaction
	int r = mdb_txn_begin(env->env, NULL, 0, &env->txn);
	if (r) {
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
		mdb_txn_commit(env->txn);
		return r;
	}

	while (mdb_cursor_get(cursor, &dbkey, &dbval, MDB_NEXT) == MDB_SUCCESS) {
		if (dbval.mv_size < sizeof(DEVICEID))
			continue;
		DEVADDR key1;
		memmove(&key1, dbkey.mv_data, sizeof(DEVADDR));
		DEVICEID v;
		memmove(&v, dbval.mv_data, sizeof(DEVICEID));
		if (onRecord(onRecordEnv, &key1, &v))
			break;
	}

	r = mdb_txn_commit(env->txn);
	if (r) {
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
		mdb_txn_commit(env->txn);
		return r;
	}
	r = mdb_cursor_get(cursor, &dbkey, &dbval, MDB_SET_RANGE);
	if (r != MDB_SUCCESS) {
		mdb_txn_commit(env->txn);
		return r;
	}

	int cnt = 0;
	do {
		if ((dbkey.mv_size < sizeof(DEVADDR)) || (dbval.mv_size < sizeof(DEVICEID)))
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
		return ERR_CODE_LMDB_TXN_COMMIT;
	}
	if (r == 0)
		r = cnt;
	return r;
}

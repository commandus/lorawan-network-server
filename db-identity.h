#ifndef DB_IDENTITY_H
#define DB_IDENTITY_H 1

#include <string>

#ifdef USE_LMDB
#include <lmdb.h>
#endif
#ifdef USE_MDBX
#include <mdbx.h>
#endif

#include "utillora.h"

#ifdef USE_MDBX
#define MDB_SET_RANGE MDBX_SET_RANGE
#define MDB_FIRST MDBX_FIRST
#define MDB_NEXT MDBX_NEXT
#define MDB_PREV MDBX_PREV
#define MDB_SUCCESS MDBX_SUCCESS
#define MDB_NOTFOUND MDBX_NOTFOUND
#define MDB_BUSY MDBX_BUSY
#define MDB_cursor_op MDBX_cursor_op
#define MDB_env	MDBX_env
#define MDB_dbi	MDBX_dbi
#define MDB_txn	MDBX_txn
#define MDB_cursor	MDBX_cursor
#define MDB_val	MDBX_val
#define mdb_env_create	mdbx_env_create
#define mdb_env_open mdbx_env_open
#define mdb_env_close mdbx_env_close
#define mdb_txn_begin mdbx_txn_begin
#define mdb_txn_commit mdbx_txn_commit
#define mdb_txn_abort mdbx_txn_abort
#define mdb_strerror mdbx_strerror
#define mdb_dbi_open mdbx_dbi_open
#define mdb_dbi_close mdbx_dbi_close
#define mdb_put mdbx_put
#define mdb_del mdbx_del
#define mdb_cursor_open mdbx_cursor_open
#define mdb_cursor_get mdbx_cursor_get
#define mdb_cursor_del mdbx_cursor_del
#define MDB_envinfo MDBX_envinfo
#define mdb_env_info mdbx_env_info
#define mdb_env_set_mapsize mdbx_env_set_mapsize
#define me_mapsize mi_mapsize
#define mv_size iov_len
#define mv_data iov_base
#define MDB_MAP_FULL MDBX_MAP_FULL
#endif

#ifdef USE_MDBX or USE_LMDB
/**
 * @brief LMDB environment(transaction, cursor)
 */
class dbenv {
public:	
	MDB_env *env;
	MDB_dbi dbi;
	MDB_txn *txn;
	MDB_cursor *cursor;
	// open db options
	std::string path;
	int flags;
	int mode;
	dbenv(const	std::string &path, int flags, int mode);
};

/**
 * @brief Opens LMDB database file
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb
(
	dbenv *env
);

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb
(
	dbenv *env
);

// callback, return true - stop request, false- continue
typedef bool (*OnRecord)
(
	void *env,
	DEVADDR *key,
	DEVICEID *data
);

/**
 * @brief Store address to the LMDB
 * @param env database env
 * @return 0 - success
 */
int putAddr
(
	dbenv *env,
	DEVADDR &addr,
	DEVICEID &deviceid
);

/**
 * @brief Get address
 * @param env database env
 * @param DEVADDR Networh address
 * @param onLog callback
 * @param onLogEnv object passed to callback
 */
int getAddr
(
	dbenv *env,
	const DEVADDR &addr,
	DEVICEID &retval
);

/**
 * @brief List address
 * @param env database env
 * @param onRecord callback
 * @param onRecordEnv object passed to callback
 */
int lsAddr
(
	dbenv *env,
	OnRecord onRecord,
	void *onRecordEnv
);

/**
 * @brief remove address from the LMDB
 * @param env database env
 * @param DEVADDR Networh address
 */
int rmAddr
(
	dbenv *env,
	const DEVADDR &addr
);

#endif
#endif

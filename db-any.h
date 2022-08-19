#ifndef DB_ANY_H_
#define DB_ANY_H_	1

#include "db-intf.h"
#include "pkt2/database-config.h"
#include "receiver-queue-service.h"

#ifdef ENABLE_PKT2
#include "pkt2/str-pkt2.h"
#endif

/**
 * Return SQL dialect number by the name
 * @param name SQL dialect name
 * @return SQL dialect number
 */
int sqlDialectByName(const std::string &name);

class DatabaseNConfig
{
public:
	const ConfigDatabase *config;
	DatabaseIntf* db;
    std::string lastErroneousStatement;

	DatabaseNConfig(const ConfigDatabase *aConfig);
	~DatabaseNConfig();
	std::string tableName(void *env, const std::string &message) const;
	std::string selectClause(void *env, const std::string &message);
	std::string createClause(void *env, const std::string &message) const;
	int insertClauses(std::vector<std::string> &retClauses,
		void *env,
		const std::string &message,
		int inputFormat,
		const std::string &data,
        const std::map<std::string, std::string> *properties,
		const std::string &nullValueString = "8888"
	);
	int createTable(void *env, const std::string &message);
	int insert(
		void *env,
		const std::string &message,
		int inputFormat,
		const std::string &data,
		const std::map<std::string, std::string> *properties,
		const std::string &nullValueString = "8888"
	);
	int open();
	int close() const;
	int exec(const std::string &statement) const;
	int select(std::vector<std::vector<std::string>> &retval, const std::string &statement);
    void setProperties(
        std::map<std::string, std::string> &retval,
        const std::map<std::string, std::string> &values
    ) const;
};

/**
 */
class DatabaseByConfig
{
private:
	const ConfigDatabasesIntf *config;
protected:
	DatabaseIntf* open(const ConfigDatabase *dbc) const;
public:
	DatabaseByConfig(const ConfigDatabasesIntf *config);
	~DatabaseByConfig();

    /**
     * Prepare received packet. Device can send two or more packets.
     * Preparation means collect packets. After all packets received, collector returns value to be inserted into the database.
     * @param env logger packet collector descriptor.  Receives and logger passport descriptor returned by initLoggerParser()
     * @param value received value payload and address
     */
    void prepare(void *env, const ReceiverQueueValue &value);

    /**
     * Prepare received packet. Device can send two or more packets.
     * Preparation means collect packets. After all packets received, collector returns value to be inserted into the database.
     * @param env logger packet collector descriptor.  Receives and logger passport descriptor returned by initLoggerParser()
     * @param addr LoRaWAN address
     * @param payload received value payload
     */
    void prepare(void *env, uint32_t addr, const std::string &payload);

    size_t count() const;
	/**
	 * @param retConfig return config
	 * @param id 0..count() - 1
	 */ 
	DatabaseIntf* getDb(const ConfigDatabase **retConfig, int seqno) const;
	DatabaseIntf* findDb(const ConfigDatabase **retConfig, const std::string &name) const;

	DatabaseNConfig *get(int id) const;
	DatabaseNConfig *find(const std::string &name) const;

	/**
	 * @param retval return dayavase identifier
	 * @return  count of databases
	 */
	size_t getIds(std::vector<int> &retval);
};

#endif

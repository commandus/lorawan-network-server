/**
 * 
 * Queue processor (in separate thread)
 * 
 * Usage:
 * 
 * // create queue
 * receiverQueueService = new JsonFileReceiverQueueService();
 * // create processor to serve queue
 * receiverQueueProcessor = new ReceiverQueueProcessor();
 * // create protobuf declarations
 * void *pkt2env = initPkt2("proto", 0);
 * receiverQueueProcessor->setPkt2Env(pkt2env);
 * // load database config
 * ConfigDatabases configDatabases("dbs.js");
 * // create helper object
 * DatabaseByConfig *dbByConfig = new DatabaseByConfig(&configDatabases);
 * // add helper object to the processor
 * receiverQueueProcessor->setDatabaseByConfig(dbByConfig);
 * // run processor
 * receiverQueueProcessor->start(receiverQueueService);
 * // stop processor
 * receiverQueueProcessor->stop();
 * 
 */
#include "receiver-queue-processor.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

#include "pkt2/str-pkt2.h"
// #include "pkt2/database-config.h"

#include <sys/time.h>
#include <unistd.h>

int ReceiverQueueProcessor::onPacket(
        struct timeval &time,
        DeviceId id,
        SemtechUDPPacket &value
)
{
	std::stringstream ss;
	ss << timeval2string(time) << MSG_DEVICE_EUI << DEVEUI2string(id.deviceEUI) << ", " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress);
	if (onLog)
		onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

	return 0;
}

ReceiverQueueProcessor::ReceiverQueueProcessor()
	: isStarted(false), isDone(false), threadDb(NULL), onLog(NULL), 
	pkt2env(NULL), databaseByConfig(NULL)
{
}

ReceiverQueueProcessor::~ReceiverQueueProcessor()
{
}

void ReceiverQueueProcessor::setLogger(
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) {
	onLog = value;
}

void ReceiverQueueProcessor::setPkt2Env
(
	void *value
)
{
	pkt2env = value;
}

void ReceiverQueueProcessor::setDatabaseByConfig
(
	DatabaseByConfig *value
)
{
	databaseByConfig = value;
}

void ReceiverQueueProcessor::start(
	ReceiverQueueService *rQueueService
)
{
	if (isStarted)
		return;
	isStarted = true;
	isDone = false;
	receiverQueueService = rQueueService;
	std::vector<int> ids;
	this->databaseByConfig->getIds(ids);
	receiverQueueService->setDbs(ids);
	threadDb = new std::thread(&ReceiverQueueProcessor::runner, this);
}

void ReceiverQueueProcessor::stop()
{
	if (!isStarted)
		return;
	isStarted = false;
	while(!isDone) {
		usleep(DB_MIN_DELAY_MS);
	}
}

void ReceiverQueueProcessor::runner()
{
	while (isStarted) {
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = DB_DEF_TIMEOUT_MS * 1000;
		int r = select(0, NULL, NULL, NULL, &timeout);
		switch (r) {
			case -1:
				break;
			case 0:
				// timeout
				processQueue();
				continue;
			default:
				break;
		}
	}
	isDone = true;
}

/**
 * Called from runner()
 */ 
void ReceiverQueueProcessor::processQueue()
{
	if (!pkt2env)
		return;
	if (!databaseByConfig)
		return;
	if (!receiverQueueService)
		return;
	size_t cnt = receiverQueueService->count();
	if (cnt > MAX_QUEUE_CNT_PER_STEP)
		cnt = MAX_QUEUE_CNT_PER_STEP;
	for (int i = 0; i < cnt; i++) {	// do not use while(!receiverQueueService->count()) to prevent endless loop
		ReceiverQueueEntry entry;
		for (int i = 0; i < databaseByConfig->count(); i++) {
			DatabaseNConfig *db = databaseByConfig->get(i);
            if (!db) {
				if (onLog) {
					std::stringstream ss;
					ss <<  ERR_DB_DATABASE_NOT_FOUND << " " << i;
					onLog(NULL, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
				}
				continue;
			}

            int dbId = db->config->id;
            bool hasDbId = receiverQueueService->peek(dbId, entry) == 0;
            if (hasDbId && db->config->active) {
                int r = db->open();
                if (r && onLog) {
                    std::stringstream ss;
                    ss << ERR_DB_DATABASE_OPEN << db->config->name << " " << r << ": " << strerror_lorawan_ns(r);
                    onLog(NULL, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
                    continue;
                }

                std::map<std::string, std::string> properties;
                entry.setProperties(properties, db->config->properties);

                r = db->insert(pkt2env, "", INPUT_FORMAT_BINARY, entry.value.payload, &properties);

                if (onLog) {
                    std::stringstream ss;
                    std::string clause = db->insertClause(pkt2env, "", INPUT_FORMAT_BINARY, entry.value.payload,
                                                          &properties);
                    if (r) {
                        ss << ERR_DB_INSERT << r
                           << " database id " << db->config->id << " " << db->config->name
                           << ": " << db->db->errmsg
                           << ", SQL statement: " << clause
                           << ", payload: " << hexString(entry.value.payload);
                        onLog(this, LOG_ERR, LOG_PACKET_HANDLER, r, ss.str());
                    } else {
                        ss << MSG_DB_INSERT
                           << " database id " << db->config->id << " " << db->config->name
                           << ", SQL statement: " << clause;
                        onLog(this, LOG_DEBUG, LOG_PACKET_HANDLER, 0, ss.str());
                    }
                    db->close();
                }
            }
			// remove database from queue if database connection is ok
            if (hasDbId) {
                if (receiverQueueService->pop(dbId, entry) != 0) {
                    if (onLog) {
                        std::stringstream ss;
                        ss << "Database: " << dbId << " " << db->config->name << " pop() error";
                        onLog(this, LOG_ERR, LOG_PACKET_HANDLER, 0, ss.str());
                    }
                }
            }
		}
	}

}

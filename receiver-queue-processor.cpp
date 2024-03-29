/**
 * 
 * Payload queue processor (in separate thread)
 * 
 * Usage:
 * 
 * // create queue
 * receiverQueueService = new JsonFileReceiverQueueService();
 * // create processor to serve queue
 * receiverQueueProcessor = new ReceiverQueueProcessor();
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
#include <sstream>

#ifdef _MSC_VER
#include <usleep.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "receiver-queue-processor.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"


#include "logger-huffman/logger-parse.h"
#include "utilthread.h"

int ReceiverQueueProcessor::onPacket(
        struct timeval &time,
        DeviceId id,
        SemtechUDPPacket &value
)
{
	std::stringstream ss;
	ss << timeval2string(time) << MSG_DEVICE_EUI << DEVEUI2string(id.devEUI) << ", " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress);
	if (onLog)
		onLog->onInfo(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
	return 0;
}

ReceiverQueueProcessor::ReceiverQueueProcessor()
	: isStarted(false), isDone(true), threadDb(nullptr), onLog(nullptr),
    databaseByConfig(nullptr)
{
}

ReceiverQueueProcessor::~ReceiverQueueProcessor()
{
}

void ReceiverQueueProcessor::setLogger(
	LogIntf *value
)
{
	onLog = value;
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
	receiverQueueService = rQueueService;
	std::vector<int> ids;
	this->databaseByConfig->getIds(ids);
	receiverQueueService->setDbs(ids);
	threadDb = new std::thread(&ReceiverQueueProcessor::runner, this);
    setThreadName(threadDb, MODULE_NAME_RECEIVER_QUEUE_PROCESSOR);
    threadDb->detach();
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
    isDone = false;
    while (isStarted) {
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = DB_DEF_TIMEOUT_MS * 1000;

		int r = select(0, nullptr, nullptr, nullptr, &timeout);
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
 * Called from processQueue()
 */
void ReceiverQueueProcessor::put2databases() {
    if (!databaseByConfig)
        return;
    if (!receiverQueueService)
        return;

    bool prepared = false;
    for (int j = 0; j < databaseByConfig->count(); j++) {
        DatabaseNConfig *db = databaseByConfig->get(j);
        if (!db) {
            if (onLog) {
                std::stringstream ss;
                ss << ERR_DB_DATABASE_NOT_FOUND << " " << j;
                onLog->onInfo(nullptr, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
            }
            continue;
        }

        int dbId = db->config->id;
        ReceiverQueueEntry entry;
        bool hasDbId = receiverQueueService->peek(dbId, entry) == 0;
        if (hasDbId && db->config->active) {
            int r = db->open();
            if (r && onLog) {
                std::stringstream ss;
                ss << ERR_DB_DATABASE_OPEN << db->config->name << " " << r << ": " << strerror_lorawan_ns(r);
                onLog->onInfo(nullptr, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
                continue;
            }

            std::map<std::string, std::string> properties;
            entry.setProperties(properties, db->config->properties);

            if (!prepared) {
                databaseByConfig->prepare(entry.value);
                if (onLog) {
                    std::stringstream ss;
                    ss << MSG_PREPARE << db->config->id << " (" << db->config->name
                       << ") ";
                    if (!db->db->errmsg.empty()) {
                        ss << ERR_MESSAGE << db->db->errmsg;
                    }
                    ss
                       << ": " << hexString(entry.value.payload);
                    onLog->onInfo(this, LOG_DEBUG, LOG_PACKET_HANDLER, r, ss.str());
                }
                prepared = true;
            }

            r = db->insert("", 0, entry.value.payload, &properties);
            if (onLog) {
                std::stringstream ss;
                if (r) {
                    ss << ERR_DB_INSERT << r
                       << " database id " << db->config->id << " " << db->config->name
                       << ": " << db->db->errmsg
                       << ", SQL statement: " << db->lastErroneousStatement
                       << ", payload: " << hexString(entry.value.payload);
                    onLog->onInfo(this, LOG_ERR, LOG_PACKET_HANDLER, r, ss.str());
                } else {
                    ss << MSG_DB_INSERT
                       << ". " << MSG_DATABASE << db->config->id << " " << db->config->name;
                    onLog->onInfo(this, LOG_DEBUG, LOG_PACKET_HANDLER, 0, ss.str());
                }
            }
            db->close();
        }
        // remove database record from queue if database connection is ok
        if (hasDbId) {
            if (receiverQueueService->pop(dbId, entry) != 0) {
                if (onLog) {
                    std::stringstream ss;
                    ss << MSG_DATABASE << dbId << " " << db->config->name << " pop() error";
                    onLog->onInfo(this, LOG_ERR, LOG_PACKET_HANDLER, 0, ss.str());
                }
            }
        }
    }
}

/**
 * Called from runner()
 */ 
void ReceiverQueueProcessor::processQueue()
{
	if (!receiverQueueService)
		return;
	size_t cnt = receiverQueueService->count();
	if (cnt > MAX_QUEUE_CNT_PER_STEP)
		cnt = MAX_QUEUE_CNT_PER_STEP;
	for (int i = 0; i < cnt; i++) {	// do not use while(!receiverQueueService->count()) to prevent endless loop
        put2databases();
	}
}

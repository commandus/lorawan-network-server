#include <algorithm>

#include "receiver-queue-service.h"
#include "base64/base64.h"

ReceiverQueueKey::ReceiverQueueKey()
	: id(0)
{

}

ReceiverQueueKey::ReceiverQueueKey
(
	const ReceiverQueueKey &value
)
	: time(value.time), id(value.id)
{

}

ReceiverQueueValue::ReceiverQueueValue
(
	const ReceiverQueueValue &value
)
	: payload(value.payload)
{

}

ReceiverQueueValue::ReceiverQueueValue()
	: payload("")
{

}

std::string ReceiverQueueValue::jsonPayload() const
{
	return base64_encode(payload, false);
}

void ReceiverQueueValue::setJsonPayload
(
	const std::string &jsonValue
)
{
	payload = base64_decode(jsonValue, true);
}

/**
 * @return remaining database count
 */
int ReceiverQueueValue::popDbId
(
	int dbid
)
{
	;
	for (std::vector<int>::iterator it(dbids.begin()); it != dbids.end(); it++)
	{
		if (*it == dbid) {
        	dbids.erase(it);
			break;
		}
    }
	return dbids.size();
}	

bool ReceiverQueueKeyCompare::operator() (
	const ReceiverQueueKey& l,
	const ReceiverQueueKey& r
) const 
{
	if (l.time.tv_sec < r.time.tv_sec)
		return true;
	if (l.time.tv_sec > r.time.tv_sec)
		return false;

	if (l.time.tv_usec < r.time.tv_usec)
		return true;
	if (l.time.tv_usec > r.time.tv_usec)
		return false;

	return l.id < r.id;
}

ReceiverQueueEntry::ReceiverQueueEntry()
{

}

ReceiverQueueEntry::ReceiverQueueEntry
(
	const ReceiverQueueKey &akey,
	const ReceiverQueueValue &avalue
)
{
	key = akey;
	value = avalue;
}

void ReceiverQueueEntry:: set(
	const ReceiverQueueKey &akey,
	const ReceiverQueueValue &avalue
) {
	key = akey;
	value = avalue;
}

ReceiverQueueService::ReceiverQueueService()
	: cnt(0)
{
}

int ReceiverQueueService::next()
{
	cnt++;
	return cnt;
}

void ReceiverQueueService::setDbs
(
	const std::vector<int> &values
)
{
	std::vector<int> d = dbs;
	for (std::vector<int>::iterator it(d.begin()); it != d.end(); it++) {
		if (std::find(values.begin(), values.end(), *it) != values.end()) {
			d.erase(it);
		}
	}
	// delete old ids not in new one
	clearDbs(d);
	dbs = values;
}

void ReceiverQueueService::push
(
	const std::string &payload,
	const timeval &time
)
{
	ReceiverQueueEntry e;
	e.key.id = next();
	e.key.time = time;
	e.value.payload = payload;
	e.value.dbids = dbs;
	pushEntry(e);
}

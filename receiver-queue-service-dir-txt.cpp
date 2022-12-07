#include <fstream>
#include <sstream>
#include <base64/base64.h>

#include "receiver-queue-service-dir-txt.h"

#include "utilstring.h"
#include "utilfile.h"
#include "errlist.h"

/**
 * Implementation of ReceiverQueueService abstract class.
 * Read payload from *.bin, *.hex, *.b64 files from specified directory.
 * Set receive time from the file modification time.
 */ 
#define CNT_FILE_EXT 3
static std::string dataFileExtensions[CNT_FILE_EXT] = {
	".bin", ".hex", ".b64"
};

DirTxtReceiverQueueServiceOptions::DirTxtReceiverQueueServiceOptions()
	: format(DIRTXT_FORMAT_BIN)
{

}

DirTxtReceiverQueueService::DirTxtReceiverQueueService()
	: writeFormat(DIRTXT_FORMAT_BIN), path("")
{

}

/**
 * Read payload from the file
 * @return payload and last modification time
 */
int DirTxtReceiverQueueService::loadFile(
	std::string &payload,
	time_t &retTime, 
	DIRTXT_FORMAT format,
	const std::string &path
)
{
	payload = file2string(path.c_str());
	switch(format) {
		case DIRTXT_FORMAT_HEX:
			// hex
			payload = hex2string(payload);
			break;
		case DIRTXT_FORMAT_BASE64:
			try {
				// base64
				payload = base64_decode(payload, false);
			}
			catch (const std::exception&) {
				return ERR_CODE_INVALID_BASE64;
			}
			break;
		default:
			// as-is binary file
			break;
	}
	if (payload.size()) {
		retTime = fileModificationTime(path);
		return 0;
	}
	return ERR_CODE_INVALID_PACKET;
}

/**
 * Write payload to the file
 * @return payload and last modification time
 */
int DirTxtReceiverQueueService::storeFile
(
	const std::string &payload,
	DIRTXT_FORMAT format
)
{
	std::string fpayload;
	switch(format) {
		case DIRTXT_FORMAT_HEX:
			// hex
			fpayload = hexString(payload);
			break;
		case DIRTXT_FORMAT_BASE64:
			// base64
			fpayload = base64_encode(payload, false);
			break;
		default:
			// as-is binary file
			format = DIRTXT_FORMAT_BIN;
			fpayload = payload;
			break;
	}
	std::stringstream fn;
	time_t t(time(NULL));


	fn << path << "/" << t << "-" << cnt << dataFileExtensions[(int)format];
	string2file(fn.str(), fpayload);
	return 0;
}

int DirTxtReceiverQueueService::count()
{
	int r = 0;
	std::vector<std::string> files;
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++) 
	{
		util::filesInPath(path, dataFileExtensions[fmt], 0, &files);
		r += files.size();
		files.clear();
	}
	return r;
}

/**
 * Get message by number
 * @return 0 if success
 */
int DirTxtReceiverQueueService::get(
	int onum,
	ReceiverQueueEntry &retval
)
{
	int r = 0;
	std::vector<std::string> files;
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++) 
	{
		util::filesInPath(path, dataFileExtensions[fmt], 0, &files);
		r += files.size();
		if (r > onum)
		{
			if (loadFile(retval.value.payload, retval.key.time.tv_sec, (DIRTXT_FORMAT) fmt, files[onum])) {
				// error
				return ERR_CODE_INVALID_PACKET;
			} else {
				retval.key.id = onum;
				retval.key.time.tv_sec = fileModificationTime(files[onum]);
				retval.key.time.tv_usec = 0;
			}
			break;
		}
	}
	return 0;
}

// Force remove entry
void DirTxtReceiverQueueService::remove(
	int onum
)
{
	int r = 0;
	std::vector<std::string> files;
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++) 
	{
		util::filesInPath(path, dataFileExtensions[fmt], 0, &files);
		r += files.size();
		if (r > onum)
		{
			if (!util::rmFile(files[onum])) {
				// TODO
			}
			break;
		}
	}
}

// Remove all entries older than parameter, if 0- remove all
void DirTxtReceiverQueueService::clear(
	time_t olderthan
)
{
	std::vector<std::string> files;
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++) 
		util::filesInPath(path, dataFileExtensions[fmt], 0, &files);
	for (std::vector<std::string>::const_iterator it(files.begin()); it != files.end(); it++)
	{
		if (fileModificationTime(*it) < olderthan)
		{
			if (!util::rmFile(*it)) {
				// TODO
			}
		}
	}
}

// Remove all entries
void DirTxtReceiverQueueService::clear()
{
	std::vector<std::string> files;
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++) 
		util::filesInPath(path, dataFileExtensions[fmt], 0, &files);
	for (std::vector<std::string>::const_iterator it(files.begin()); it != files.end(); it++)
	{
		if (!util::rmFile(*it)) {
			// TODO
		}
	}
}

// Remove all entries with databases
void DirTxtReceiverQueueService::clearDbs(
	const std::vector <int> &dbs
)
{
	// TODO
	clear();
}

// Add entry
void DirTxtReceiverQueueService::pushEntry(
	ReceiverQueueEntry &value
)
{
	storeFile(value.value.payload, writeFormat);
}

/** 
 * @return 0 if success, ERR_CODE_QUEUE_EMPTY if no message in queue, ERR_CODE_RM_FILE if program can nor delete file
 * @param dbid ignored
 */
int DirTxtReceiverQueueService::pop(
	const int &dbid,
	ReceiverQueueEntry &retval
)
{
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++)
	{
		std::vector<std::string> f;
		util::filesInPath(path, dataFileExtensions[fmt], 0, &f);
		std::vector<std::string>::const_iterator it(f.begin());
		if (it != f.end())
		{
			if (loadFile(retval.value.payload, retval.key.time.tv_sec, (DIRTXT_FORMAT) fmt, *it)) {
				// error
				continue;
			} else {
				ReceiverQueueEntry e;
				retval.key.id = 0;
				retval.key.time.tv_sec = fileModificationTime(*it);
				retval.key.time.tv_usec = 0;
				if (!util::rmFile(*it))
					return ERR_CODE_RM_FILE;
				return 0;
			}
		}
	}
	return ERR_CODE_QUEUE_EMPTY;
}

/** 
 * @return 0 if success, ERR_CODE_QUEUE_EMPTY if no message in queue, ERR_CODE_RM_FILE if program can nor delete file
 * @param dbid ignored
 */
int DirTxtReceiverQueueService::peek(
	const int &dbid,
	ReceiverQueueEntry &retval
)
{
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++)
	{
		std::vector<std::string> f;
		util::filesInPath(path, dataFileExtensions[fmt], 0, &f);
		std::vector<std::string>::const_iterator it(f.begin());
		if (it != f.end())
		{
			if (loadFile(retval.value.payload, retval.key.time.tv_sec, (DIRTXT_FORMAT) fmt, *it)) {
				// error
				continue;
			} else {
				ReceiverQueueEntry e;
				retval.key.id = 0;
				retval.key.time.tv_sec = fileModificationTime(*it);
				retval.key.time.tv_usec = 0;
				return 0;
			}
		}
	}
	return ERR_CODE_QUEUE_EMPTY;
}

// List entries
void DirTxtReceiverQueueService::list(
	std::vector<ReceiverQueueEntry> &retval,
	size_t offset,
	size_t size
)
{
	int c = 0;
	
	for (int fmt = 0; fmt < CNT_FILE_EXT; fmt++)
	{
		ReceiverQueueEntry e;
		std::vector<std::string> f;
		util::filesInPath(path, dataFileExtensions[fmt], 0, &f);
		for (std::vector<std::string>::const_iterator it(f.begin()); it != f.end(); it++)
		{
			if (loadFile(e.value.payload, e.key.time.tv_sec, (DIRTXT_FORMAT) fmt, *it)) {
				// error
			} else {
				e.key.id = c;
				e.key.time.tv_sec = fileModificationTime(*it);
				e.key.time.tv_usec = 0;
				retval.push_back(e);
			}
			c++;
		}
	}
}

// force save
void DirTxtReceiverQueueService::flush()
{
}

// reload
int DirTxtReceiverQueueService::init(
	const std::string &option,
	void *data
)
{
	path = option;
	writeFormat = DIRTXT_FORMAT_BIN;
	if (data)
	{
		writeFormat = ((DirTxtReceiverQueueServiceOptions *) data)->format;
	}
	return 0;
}

// close resources
void DirTxtReceiverQueueService::done()
{
}

/**
 * 	JSON attribute names
 */
#define ATTRS_COUNT	4
static const char *ATTR_NAMES[ATTRS_COUNT] = {
	"time", 		// time value
	"id",			// identifier
	"payload",		// payload
	"dbids"			// database identifiers
};

std::string DirTxtReceiverQueueService::toJsonString()
{
	std::vector<ReceiverQueueEntry> entries;
	list(entries, 0, count());
	std::stringstream ss;
	ss << "[";
	bool addSeparator(false);
	for (std::vector<ReceiverQueueEntry>::const_iterator it(entries.begin()); it != entries.end(); it++) {
		if (addSeparator)
			ss << ",";
		else
			addSeparator = true;
		ss << "{\"" 
			<< ATTR_NAMES[0] << "\":" << it->key.time.tv_sec << ",\"" 
			<< ATTR_NAMES[1] << "\":" << it->key.id << ",\"" 
			<< ATTR_NAMES[2] << "\":\"" << it->value.jsonPayload() << "\"" ;
		if (it->value.dbids.size()) {
			ss << ", \"" << ATTR_NAMES[3] << "\":[";
			bool isFirst = true;
			for (std::vector<int>::const_iterator itd(it->value.dbids.begin()); itd != it->value.dbids.end(); itd++) {
				if (isFirst)
					isFirst = false;
				else
					ss << ", ";
				ss << *itd;
			}
			ss << "]";
		}
		ss << "}";
	}
	ss << "]";
	return ss.str();
}

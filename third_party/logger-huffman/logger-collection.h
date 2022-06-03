#ifndef LOGGER_COLLECTION_H_
#define LOGGER_COLLECTION_H_ 1

#include <string>
#include <vector>
#include <map>

#include "logger-huffman.h"

// следующий пакет typ 4b- plain 49- delta 4d- huffman
class LoggerItemId {
	public:
		uint8_t kosa;							// идентификатор косы (номер, дата)
		uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
		int8_t packet;							// packet number
		uint8_t kosa_year;						// reserved for first packet

		LoggerItemId();
		LoggerItemId(
			uint8_t kosa,						// идентификатор косы (номер, дата)
			uint8_t measure,					// мл. Байт номера замера, lsb used (или addr_used?)
			uint8_t packet,						// packet number
			uint8_t kosa_year
		);

		/**
		 * Set identifier
		 * @param akosa kosa number
		 * @param ameasure measurement no
		 * @param apacket -1- first packet (w/o data)
		 */ 
		void set(
			uint8_t kosa,						// идентификатор косы (номер, дата)
			uint8_t measure,					// мл. Байт номера замера, lsb used (или addr_used?)
			int8_t packet,						// packet number
			uint8_t kosa_year
		);
		void assign(
			LOGGER_MEASUREMENT_HDR *retval
		);

		LoggerItemId& operator=(const LoggerItemId& other);
		bool operator==(const LoggerItemId &another) const;
		bool operator!=(const LoggerItemId &another) const;

		std::string toString() const;
		std::string toJsonString() const;

        std::string kosaString() const;							// идентификатор косы (номер, дата)
        std::string measureString() const;						// мл. Байт номера замера, lsb used (или addr_used?)
        std::string packetString() const;						// packet number
        std::string kosaYearString() const; 					// reserved for first packet
        void set(const LOGGER_MEASUREMENT_HDR &param);

    void clear();
};

class LoggerCollection;

/**
 * Keep measurements
 */
class LoggerItem {
private:
    /**
     * Debug print out delta aPacket 1
     * @param aPacket delta aPacket
     * @return
     */
    std::string delta1ToString(const std::string &aPacket) const;
    /**
     * Debug print out delta packet 2
     * @param packet delta packet
     * @return
     */
    std::string delta2ToString(const std::string &packet) const;
    /**
     * Debug print out JSON delta packet 1
     * @param aPacket
     * @return JSON string
     */
    std::string delta1ToJson(const std::string &aPacket) const;
    /**
     * Debug print out JSON delta packet 2
     * @param aPacket
     * @return JSON string
     */
    std::string delta2ToJson(const std::string &aPacket) const;

    /**
     * return temperature by diff
     * @param retVal return temperature by diff
     * @return temperature by diff
     */
    bool getByDiff(std::map<uint8_t, TEMPERATURE_2_BYTES> *retVal, std::map<uint8_t, double> *retValT,
        const std::string &aPacket, int packetNo) const;
protected:
    /**
     * Return approximated temperature
     * @param sensor sensor index
     * @param value temperature, C
     * @return approximated temperature
     */
    double correctTemperatureByPassport(uint8_t sensor, double value) const;
public:
    LoggerItemId id;
    std::string packet;
    time_t parsed{};
    uint32_t addr;  // source address
    int errCode;
    // owner
    LoggerCollection *collection;
    LOGGER_MEASUREMENT_HDR *getMeasurementHeaderIfExists() const;
    LoggerItem();
    LoggerItem(LoggerCollection *collection);
    LoggerItem(time_t t);
    LoggerItem(const LoggerItem &value);
    LoggerItem(uint32_t addr, const void *aBuffer, size_t aSize);
    virtual ~LoggerItem();

    LoggerItem& operator=(const LoggerItem& other);
    bool operator==(const LoggerItem &another) const;
    bool operator==(const LoggerItemId &id) const;

    bool operator!=(const LoggerItem &another) const;
    bool operator!=(const LoggerItemId &id) const;

    LOGGER_PACKET_TYPE set(uint8_t &retPackets, size_t &retSize, uint32_t addr, const void *aBuffer, size_t aSize);
    bool get(std::map<uint8_t, TEMPERATURE_2_BYTES> &retval) const;
    bool getTemperature(std::map<uint8_t, double> &t) const;
    bool getCorrectedTemperature(std::map<uint8_t, double> &t) const;

    std::string toString() const;
    std::string toJsonString() const;
    std::string toTableString() const;

    bool setMeasurementHeaderFromDiffIfExists();

    int getDataBytes() const;

    /**
     * Return kosa year from first packet if exists or base record loaded by LoraWAN device address
     * Second packet header have kosa number but no kosa year. Restore kosa year.
     * @return 0 if not exists
     */
    uint8_t getKosaYearFromFirstPacketOrLoad();
};

/**
 * keep LOGGER_MEASUREMENT_HDR
 */
class LoggerMeasurementHeader {
public:
	const LOGGER_MEASUREMENT_HDR *header;
    LoggerItemId id;
    time_t start;
    uint8_t vcc;
    uint8_t vbat;

    LoggerMeasurementHeader();
    LoggerMeasurementHeader(const LoggerMeasurementHeader &value);
    LoggerMeasurementHeader(const LOGGER_MEASUREMENT_HDR *aHeader, size_t sz);

    LoggerMeasurementHeader& operator=(const LOGGER_MEASUREMENT_HDR &value);
    bool operator==(const LoggerItemId &another) const;
    bool operator==(const LoggerMeasurementHeader &value) const;

    bool operator!=(const LoggerItemId &another) const;
    bool operator!=(const LoggerMeasurementHeader &value) const;

    bool setHdr(const LOGGER_MEASUREMENT_HDR *pHeader, size_t sz);

    void assign(LOGGER_MEASUREMENT_HDR &retval) const;
};

class LoggerKosaPackets;
class LoggerKosaCollector;

/** 
 * Raw collection of packets
 */
class LoggerCollection {
	public:
        std::vector<LoggerItem> items;
		uint8_t expectedPackets;	// keep expected packets
		int errCode;
        LoggerKosaCollector *collector;
        // kosa owns packet items
        LoggerKosaPackets *kosa;
        // point to the first header

		LoggerCollection();
        LoggerCollection(LoggerKosaCollector *aCollector);
        LoggerCollection(const LoggerCollection &value);
		virtual ~LoggerCollection();

		void push(LoggerItem &value);
        /**
          * Put char buffer
          */
        LOGGER_PACKET_TYPE put(size_t &retSize, std::vector<LOGGER_MEASUREMENT_HDR> *retHeaders, uint32_t addr,
                               const void *buffer, size_t aSize);
		/**
		 * Put collection of strings
		 */
        LOGGER_PACKET_TYPE put(std::vector<LOGGER_MEASUREMENT_HDR> *retHeaders, uint32_t addr,
                               const std::vector<std::string> &values);

		bool completed() const;
		bool get(std::map<uint8_t, TEMPERATURE_2_BYTES> &retval) const;
        bool getTemperature(std::map<uint8_t, double> &retval) const;
        bool getCorrectedTemperature(std::map<uint8_t, double> &t) const;
        /**
         * Get bits size 1 or 2 bits in
         * @return bullptr if not exists
         */
        LOGGER_PACKET_FIRST_HDR *getFirstHeader();

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString(const LoggerItemId &id, const time_t &t, const LOGGER_MEASUREMENT_HDR &header) const;
private:
    LOGGER_PACKET_TYPE put1(size_t &retSize, std::vector<LOGGER_MEASUREMENT_HDR> *retHeaders, uint32_t addr,
                            const void *buffer, size_t size);
    void putRaw(size_t &retSize, uint32_t addr, const void *buffer, size_t size);

};

//  5'
#define MAX_SECONDS_WAIT_KOSA_PACKETS 5 * 60

class LoggerKosaCollector; // forward declaration

/** 
 * Kosa packets collection
 */
class LoggerKosaPackets {
private:
    // base record for diff, loaded by loadBaseKosa() from collection (if set)
    LoggerKosaPackets *baseKosa;
public:
    // parent provides access to the passport
    LoggerKosaCollector *collector;
    LoggerItemId id;
    time_t start;
    LOGGER_MEASUREMENT_HDR measurementHeader;
    LoggerCollection packets;

    LoggerKosaPackets();
    LoggerKosaPackets(LoggerKosaCollector *aCollector);
    LoggerKosaPackets(const LoggerKosaPackets &value);
    LoggerKosaPackets(const LoggerItem &value);
    virtual ~LoggerKosaPackets();

    bool expired() const;

    bool add(LoggerItem &value);

    // do not nodeCompare with packet!
    bool operator==(const LoggerItemId &another) const;
    bool operator!=(const LoggerItemId &another) const;

    bool operator==(uint8_t kosa) const;
    bool operator!=(uint8_t kosa) const;

    time_t measured() const;

    std::string toString() const;
    /**
     * Return more detailed, packet by packet string
     * @return text
     */
    std::string packetsToString() const;
    std::string toJsonString() const;
    /**
     * Return detailed JSON string, packet by packet
     * @return detailed JSON string, packet by packet
     */
    std::string packetsToJsonString() const;
    std::string toTableString() const;

    /**
     * List of values as 2-bytes hex string. (you need calc temperature yourself).
     * @param ostrm output stream to write values
     * @param separator list separator e.g. ", "
     * @param substEmptyValue substitute substEmptyValue string if sensor has no value, e.g. "null"
     */
    void valueCommaString(std::ostream &ostrm, const std::string &separator, const std::string &oparen,
                          const std::string &cparen, const std::string &substEmptyValue) const;
    /**
     * List of temperature values without correction.
     * @param ostrm output stream to write temperature values
     * @param separator list separator e.g. ", "
     * @param substEmptyValue substitute substEmptyValue string if sensor has no value, e.g. "null"
     */
    void temperatureCommaString(std::ostream &ostrm, const std::string &separator, const std::string &substEmptyValue) const;
    /**
     * List of corrected temperature values. Correction has mad by approximation using kosa passport.
     * @param ostrm output stream to write temperature values
     * @param separator list separator e.g. ", "
     * @param substEmptyValue substitute substEmptyValue string if sensor has no value, e.g. "null"
     */
    void temperatureCorrectedCommaString(std::ostream &ostrm, const std::string &separator, const std::string &substEmptyValue) const;

    /**
     * Write packets as hex string separated by separator, for instance, comma
     * @param ostrm output stream
     * @param separator separator e.g. ", "
     */
    void rawCommaString(std::ostream &ostrm, const std::string &separator, const std::string &oparen,
                        const std::string &cparen) const;

    void toStrings(std::vector<std::string> &retval, const std::string &substEmptyValue) const;

    LoggerKosaPackets *loadBaseKosa(uint32_t addr);

    void updateKosaAfterCopy();
};

class LoggerKosaPacketsLoader {
public:
    /**
     * Load last kosa record with "base" values by address
     * @param retVal set kosa packets if found
     * @param addr kosa address
     * @return true- kosa with "base" record found
     */
    virtual bool load(LoggerKosaPackets &retVal, uint32_t addr) = 0;
};

/** 
 * Make an collection of kosa
 */
class LoggerKosaCollector {
	public:
		void *passportDescriptor;
        LoggerKosaPacketsLoader *loggerKosaPacketsLoader;
		std::vector<LoggerKosaPackets> koses;

		LoggerKosaCollector();
		virtual ~LoggerKosaCollector();

		int rmExpired();

		void add(LoggerCollection &value);
		// helper functions
		/**
		 * Put char buffer
		 */
        LOGGER_PACKET_TYPE put(LoggerKosaCollector *aCollector, size_t &retSize, uint32_t addr, const void *buffer,
                               size_t size);
        /**
        * Put one string
        */
        LOGGER_PACKET_TYPE put(uint32_t addr, const std::string &value);
        /**
		 * Put collection of strings
		 */
        LOGGER_PACKET_TYPE put(uint32_t addr, const std::vector<std::string> &values);

        /**
         * Text
         * @return
         */
        std::string toString() const;
        /**
         * More details
         * @return text
         */
		std::string packetsToString() const;
        std::string toJsonString() const;
		std::string packetsToJsonString() const;
		std::string toTableString() const;
        bool addHeader(const LOGGER_MEASUREMENT_HDR &value);

		void setPassports(void *passportDescriptor);
        void setLoggerKosaPacketsLoader(LoggerKosaPacketsLoader *);
};

std::string LOGGER_PACKET_TYPE_2_string(const LOGGER_PACKET_TYPE &value);

LOGGER_PACKET_TYPE LOGGER_PACKET_TYPE_2_string(const std::string &value);

std::string LOGGER_MEASUREMENT_HDR_2_string(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_MEASUREMENT_HDR_2_json(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_MEASUREMENT_HDR_2_table(const LOGGER_MEASUREMENT_HDR &value);
std::string vcc2string(uint8_t value);
std::string vbat2string(uint8_t value);

std::string LOGGER_DATA_TEMPERATURE_RAW_2_json(const LOGGER_DATA_TEMPERATURE_RAW *value);
std::string LOGGER_DATA_TEMPERATURE_RAW_2_text(const LOGGER_DATA_TEMPERATURE_RAW *value);
std::string LOGGER_MEASUREMENT_HDR_DIFF_2_json(const LOGGER_MEASUREMENT_HDR_DIFF *value);
std::string LOGGER_MEASUREMENT_HDR_DIFF_2_string(const LOGGER_MEASUREMENT_HDR_DIFF *value);

std::string LOGGER_PACKET_FIRST_HDR_2_string(const LOGGER_PACKET_FIRST_HDR &value);
std::string LOGGER_PACKET_FIRST_HDR_2_json(const LOGGER_PACKET_FIRST_HDR &value);
std::string LOGGER_PACKET_SECOND_HDR_2_string(const LOGGER_PACKET_SECOND_HDR &value);
std::string LOGGER_PACKET_SECOND_HDR_2_json(const LOGGER_PACKET_SECOND_HDR &value);

/** hexadecimal data represented string to binary */
std::string hex2binString(const char *hexChars, size_t size);
std::string hex2binString(const std::string &value);
/** binary data to hexadecimal represented data string to binary */
std::string bin2hexString(const char *binChars, size_t size);
std::string bin2hexString(const std::string &value);

/**
 * Return error code description
 * @param errCode error code to explain
 * @return error description
 */
const char *strerror_logger_huffman(int errCode);

void clear_LOGGER_MEASUREMENT_HDR(LOGGER_MEASUREMENT_HDR &value);

#endif

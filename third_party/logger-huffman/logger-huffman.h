#ifndef LOGGER_HUFFMAN_H_
#define LOGGER_HUFFMAN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <inttypes.h>
#include <time.h>

#ifdef __GNUC__
#include <endian.h>
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif
#endif



#define MAX_SENSOR_COUNT			28
#define MAX_HDR_DATA_SIZE			24	// bytes


typedef enum {
	LOGGER_PACKET_RAW = 0,			// raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
	LOGGER_PACKET_UNKNOWN = 1,		// reserved for unknown
 	LOGGER_PACKET_PKT_1 = 0x4a,		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
 	LOGGER_PACKET_PKT_2 = 0x4b,		// with packet header (next)
 	LOGGER_PACKET_DELTA_1 = 0x48,	// дельты замеров от 0 замера.
	LOGGER_PACKET_DELTA_2 = 0x49,	// дельты замеров от 0 замера.
 	LOGGER_PACKET_HUFF_1 = 0x4c,	// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
	LOGGER_PACKET_HUFF_2 = 0x4d	// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
} LOGGER_PACKET_TYPE;

/*
 * Packet sequence:
 *
 *  00 Raw
 *   LOGGER_PACKET_RAW(16+4*) := LOGGER_MEASUREMENT_HDR(16 bytes) LOGGER_DATA_TEMPERATURE_RAW(4)
 *   LOGGER_DATA_TEMPERATURE_RAW(4) := sensor(1) temperature(2) rfu1(1)
 *  4a (first packet)
 *   LOGGER_PACKET_PKT_1(24) := LOGGER_PACKET_FIRST_HDR(8) LOGGER_MEASUREMENT_HDR(16)
 *  4b (next packets, 8..24 bytes)
 *   LOGGER_PACKET_PKT_2(4 + 4*) := LOGGER_PACKET_SECOND_HDR(4) LOGGER_DATA_TEMPERATURE_RAW(4)*
 *  48 (first diff packet with 1..6 temperature differences
 *   LOGGER_PACKET_PKT_DIFF_1(24) := LOGGER_PACKET_FIRST_HDR(8) LOGGER_MEASUREMENT_HDR_DIFF(10) LOGGER_DATA_TEMPERATURE_DIFF(1..2)*
 *  49 (next diff packet)
 *   LOGGER_PACKET_PKT_DIFF_2(24) := LOGGER_PACKET_SECOND_HDR(4) LOGGER_DATA_TEMPERATURE_DIFF(1..2)*
 */

typedef PACK( struct {
	uint8_t memblockoccupation;				// 0 0- memory block occupied
	uint8_t seconds;						// 1 0..59
	uint8_t minutes;						// 2 0..59
	uint8_t hours;							// 3 0..23
	uint8_t day;							// 4 1..31
	uint8_t month;							// 5 1..12
	uint8_t year;							// 6 0..99 year - 2000 = last 2 digits
	uint8_t kosa;							// 7 номер косы в году
	uint8_t kosa_year;						// 8 год косы - 2000 (номер года последние 2 цифры)
	uint8_t rfu1;							// 9 reserved
	uint8_t rfu2;							// 10 reserved
	uint8_t vcc;							// 11 V cc bus voltage, V
	uint8_t vbat;							// 12 V battery, V
	uint8_t pcnt;							// 13 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
	uint16_t used;							// 14 record number, 1..65535
} ) LOGGER_MEASUREMENT_HDR;			// 16 bytes

/**
 * short (diff) version of LOGGER_MEASUREMENT_HDR(15 bytes)
 * 10 bytes long
 */
typedef PACK( struct {
    int16_t used;							// 0 record number diff
    int8_t delta_sec;				        // 2 seconds
    int8_t kosa;							// 3 номер косы в году
    int8_t kosa_year;						// 4 год косы - 2000 (номер года последние 2 цифры)
    int8_t rfu1;							// 5 reserved
    int8_t rfu2;							// 6 reserved
    int8_t vcc; 							// 7 V cc bus voltage, V
    int8_t vbat;							// 8 V battery, V
    int8_t pcnt;							// 9 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
} ) LOGGER_MEASUREMENT_HDR_DIFF;		// 10 bytes

// first packet types: 4a- plain 48- delta 4c- huffman
/**
 * typ:
 * 		LOGGER_PACKET_RAW		0x00 формируется, но не отправляется, 
 *		LOGGER_PACKET_PKT_1		0х4A без сжатия, 0x4B- последующие пакеты
 *		LOGGER_PACKET_DELTA_1	0х48 просто сжатие дельта, 49 последующие пакет
 *		LOGGER_PACKET_HUFF_1	0x4c хафман, 4D последующие пакеты
 */
typedef PACK( struct {
	uint8_t typ;						    // 	LOGGER_PACKET_RAW, LOGGER_PACKET_PKT_1, LOGGER_PACKET_DELTA_1, LOGGER_PACKET_HUFF_1
	struct {
		uint8_t data_bits: 4;
		uint8_t rfu: 3;
		uint8_t command_change: 1;
	} status;                               // статус замера, биты 0-3 битовая длина тела данных замера, бит 7 – получена команда на смену 0 замера.
	uint16_t size;							// (compressed) общая длина данных, bytes
	uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
	uint8_t packets;						// количество пакетов в замере! (лора по 24 байта с шапками пакетов)
	uint8_t kosa;							// plume serial number
	uint8_t kosa_year;						// plume producation year - 2000
} ) LOGGER_PACKET_FIRST_HDR;			// 8 bytes

// следующий пакет typ 4b- plain 49- delta 4d- huffman
typedef PACK( struct {
	uint8_t typ;						    // 49 просто сжатие дельта, 0х4b 4d хафман
	uint8_t kosa;							// plume serial number
	uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
	uint8_t packet;							// номер пакета в замере
} ) LOGGER_PACKET_SECOND_HDR;			// 4 bytes

typedef PACK( struct {
	uint8_t lo;						    	// Temperature * 0.625, lo byte
	int8_t hi;								// Temperature * 0.625, hi byte
} ) LO_HI_2_BYTES;		        		// 2 bytes

typedef PACK( struct {
    int8_t h;
    int8_t l: 4;
    int8_t f: 4;
} ) TEMPERATURE_12_BITS;				// 2 bytes

typedef PACK( struct {
	union {
		int16_t t00625;						// temperature
		LO_HI_2_BYTES f;
        TEMPERATURE_12_BITS t12;
	} t;
} ) TEMPERATURE_2_BYTES;				// 2 bytes

typedef PACK( struct {
	uint8_t sensor;						    // номер датчика 0..255
	TEMPERATURE_2_BYTES value;
	uint8_t rfu1;							// angle,. not used
} ) LOGGER_DATA_TEMPERATURE_RAW;		// 4 bytes

typedef PACK( struct {
	union {
		int16_t t;							// temperature, C. 12 bits
		LO_HI_2_BYTES f;
	} value;
} ) LOGGER_DATA_TEMPERATURE;			// 2 bytes

/**
 * Return LOGGER_PACKET_UNKNOWN if buffer is NULL or size = 0
 * @param retSize return packet size if not NULL
 * @param buffer read data from the buffer
 * @param bufferSize buffer size
 */ 
LOGGER_PACKET_TYPE extractLoggerPacketType(
	size_t *retSize,
	const void *buffer,
	size_t bufferSize
);

/**
 * Extract header only
 * @param retHdr return header pointer
 * @param buffer data
 * @param size buffer size
 */
LOGGER_PACKET_TYPE extractMeasurementHeader(
	LOGGER_MEASUREMENT_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
);

/**
 * Return expected packet size in bytes
 * @param typ packet type
 * @param bufferSize size
 */
size_t getLoggerPacketTypeSize(
	LOGGER_PACKET_TYPE typ,
	size_t bufferSize
);

/**
 * Extract first header
 * @param retHdr return header pointer
 * @param buffer data
 * @param size buffer size
 * @return 0- success, <0- error code
 */
int extractFirstHdr(
	LOGGER_PACKET_FIRST_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
);

/**
 * Extract seconf header
 * @param retHdr return header pointer
 * @param buffer data. Huffman packet must decoded first
 * @param size buffer size
 * @return 0- success, <0- error code
 */
int16_t extractSecondHdr(
	LOGGER_PACKET_SECOND_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
);

double extractMeasurementHeaderData(
	LOGGER_DATA_TEMPERATURE_RAW **retval,
	int idx,
	const void *buffer,
	size_t bufferSize
);

LOGGER_DATA_TEMPERATURE_RAW *extractSecondHdrData(
	int p,
	const void *buffer,
	size_t bufferSize
);

LOGGER_MEASUREMENT_HDR_DIFF *extractDiffHdr(
    const void *buffer,
    size_t bufferSize
);

double TEMPERATURE_2_BYTES_2_double(
	TEMPERATURE_2_BYTES value
);

void double_2_TEMPERATURE_2_BYTES(
    TEMPERATURE_2_BYTES *retVal,
    double value
);

// convert used field
uint16_t LOGGER_MEASUREMENT_HDR_USED(uint16_t value);

double vcc2double(
	uint8_t value
);

double vbat2double(
	uint8_t value
);

uint8_t double2vcc(
    double value
);

uint8_t  double2vbat(
    double value
);

time_t LOGGER_MEASUREMENT_HDR2time_t(
    LOGGER_MEASUREMENT_HDR *header,
    int isLocaltime
);

time_t logger2time(
    uint8_t year2000,
    uint8_t month,
    uint8_t date,
    uint8_t hours,
    uint8_t minutes,
    uint8_t seconds,
    int isLocaltime
);

/**
 * Convert LOGGER_MEASUREMENT_HDR to the short version of header
 * @param retval return short(diff) version
 * @param h1 current header
 * @param h0 base header
 */
void LOGGER_MEASUREMENT_HDR_delta(
    LOGGER_MEASUREMENT_HDR_DIFF *retval,
    LOGGER_MEASUREMENT_HDR *h1,
    LOGGER_MEASUREMENT_HDR *h0
);

/**
 * Return diff value
 * @param buffer packet
 * @param dataSizeBytes 1 or 2 bytes per diff temperature
 * @param index zero based index
 * @return diff value
 */
int getDiff(
    const void *buffer,
    int dataSizeBytes,
    int index
);

#ifdef __cplusplus
}
#endif

#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#else
#include <unistd.h>
#endif

#include "utilstring.h"

#include <stdlib.h>
#include <algorithm> 
#include <functional> 
#include <sstream>
#include <streambuf>
#include <iomanip>
#include <fstream>
#include <climits>

static std::string file2string(
	std::istream &strm
)
{
	if (!strm)
		return "";
	return std::string((std::istreambuf_iterator<char>(strm)), std::istreambuf_iterator<char>());
}

std::string file2string(
	const char *filename
)
{
	if (!filename)
		return "";
	std::ifstream t(filename);
	return file2string(t);
}

bool string2file(
	const std::string &filename,
	const std::string &value
)
{
    FILE* f = fopen(filename.c_str(), "w");
	if (!f)
		return false;
	fwrite(value.c_str(), value.size(), 1, f);
	fclose(f);
	return true;
}

bool append2file(
	const std::string &filename,
	const std::string &value
)
{
	FILE* f = fopen(filename.c_str(), "a");
	if (!f)
		return false;
	fwrite(value.c_str(), value.size(), 1, f);
	fclose(f);
	return true;
}

// http://stackoverflow.com/questions/673240/how-do-i-print-an-unsigned-char-as-hex-in-c-using-ostream
struct HexCharStruct
{
	unsigned char c;
	HexCharStruct(unsigned char _c) : c(_c) { }
};

inline std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs)
{
	return (o << std::setfill('0') << std::setw(2) << std::hex << (int) hs.c);
}

inline HexCharStruct hex(unsigned char c)
{
	return HexCharStruct(c);
}

static void bufferPrintHex(std::ostream &ostream, const void* value, size_t size)
{
	if (value == NULL)
		return;
	unsigned char *p = (unsigned char*) value;
	for (size_t i = 0; i < size; i++)
	{
		ostream << hex(*p);
		p++;
	}
}

std::string hexString(const void *buffer, size_t size)
{
	std::stringstream r;
	bufferPrintHex(r, buffer, size);
	return r.str();
}

/**
 * Return hex string
 * @param data
 * @return
 */
std::string hexString(const std::string &data)
{
	return hexString((void *) data.c_str(), data.size());
}

static std::string readHex(std::istream &s)
{
	std::stringstream r;
	s >> std::noskipws;
	char c[3] = {0, 0, 0};
	while (s >> c[0])
	{
		if (!(s >> c[1]))
			break;
		unsigned char x = (unsigned char) strtol(c, NULL, 16);
		r << x;
	}
	return r.str();
}

std::string hex2string(const std::string &hex)
{
	std::stringstream ss(hex);
    return readHex(ss);
}

/**
 * @brief Return binary data string
 * @param hex hex string
 * @return binary data string
 */
std::string stringFromHex(const std::string &hex)
{
	std::string r(hex.length() / 2, '\0');
	std::stringstream ss(hex);
	ss >> std::noskipws;
	char c[3] = {0, 0, 0};
	int i = 0;
	while (ss >> c[0]) {
		if (!(ss >> c[1]))
			break;
		unsigned char x = (unsigned char) strtol(c, NULL, 16);
		r[i] = x;
		i++;
	}
	return r;
}

/**
 * @see https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
 */
// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

bool isHex(
	const std::string &value
) {
	return value.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}

/**
 * @see https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
 */
std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

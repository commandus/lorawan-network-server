std::string jsonPackage(
	const std::string &rfmTxPackage
)
{
	int ms;
	time_t t = time_ms(ms);
	std::string dt = ltimeString(t, ms, "%FT%T") + "Z";	// "2020-12-16T12:17:00.12345Z";

	std::stringstream ss;
	ss << "{\"rxpk\":[{ \
	\"time\":\""<< dt << "\", \
	\"tmst\":3512348611, \
	\"chan\":0, \
	\"rfch\":0, \
	\"freq\":868.900000, \
	\"stat\":1, \
	\"modu\":\"LORA\", \
	\"datr\":\"SF7BW125\", \
	\"codr\":\"4/6\", \
	\"rssi\":-35, \
	\"lsnr\":5.1, \
	\"size\":" << rfmTxPackage.size() << ", \
	\"data\":\"" << base64_encode(std::string((const char *) rfmTxPackage.c_str(), rfmTxPackage.size())) << "\" \
	}]}";
	return ss.str();
}


/**
 * @brief constructs a LoRaWAN package and sends it
 * @param data pointer to the array of data that will be transmitted
 * @param dataLength bytes to be transmitted
 * @param frameCounterUp  frame counter of upstream frames
 * @param devAddr 4 bytes long device address
 * @param nwkSkey 128 bits network key
 * @param appSkey 128 bits application key
 */
std::string loraDataJson(
	std::string &data, 
	unsigned int frameCounterTx,
	DEVADDR &devAddr,
	KEY128 &nwkSKey,
	KEY128 &appSKey
)
{
	unsigned char i;

	// direction of frame is up
	unsigned char direction = 0x00;

	unsigned char rfmData[64];
	unsigned char rfmPackageLength;

	uint32_t MIC;

	unsigned char frameControl = 0x00;
	unsigned char framePort = 0x01;

	// encrypt data
	encryptPayload(data, frameCounterTx, direction, devAddr, appSKey);

	// build radio packet
	// unconfirmed data up
	unsigned char macHeader = 0x40;

	rfmData[0] = macHeader;

	rfmData[1] = devAddr[3];
	rfmData[2] = devAddr[2];
	rfmData[3] = devAddr[1];
	rfmData[4] = devAddr[0];

	rfmData[5] = frameControl;

	rfmData[6] = (frameCounterTx & 0x00FF);
	rfmData[7] = ((frameCounterTx >> 8) & 0x00FF);

	rfmData[8] = framePort;

	// set current packet length
	rfmPackageLength = 9;

	// load data
	for (i = 0; i < data.size(); i++) {
		rfmData[rfmPackageLength + i] = data[i];
	}

	// Add data Lenth to package length
	rfmPackageLength = rfmPackageLength + data.size();

	// calc MIC
	MIC = calculateMIC(std::string((char *) rfmData, rfmPackageLength), frameCounterTx, direction, devAddr, nwkSKey);

	// load MIC in package
	memcpy(&rfmData + rfmPackageLength, &MIC, 4);

	// add MIC length to RFM package length
	rfmPackageLength = rfmPackageLength + 4;

	// make JSON package
	return jsonPackage(std::string((char *)rfmData, rfmPackageLength));
}



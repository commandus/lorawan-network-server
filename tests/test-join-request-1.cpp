#include <string>
#include <iostream>

#include "errlist.h"
#include "utilfile.h"
#include "regional-parameter-channel-plan-file-json.h"


void loadFile(RegionalParameterChannelPlanFileJson &value, const std::string &fn)
{
    value.init(fn, nullptr);
    if (value.errCode) {
        std::cerr << ERR_MESSAGE << value.errCode << ": "
                  << " " << value.errDescription << std::endl;
    }
}

void saveFile(RegionalParameterChannelPlanFileJson &value, const std::string &fn)
{
    value.flush();
}

void printRegionBands(const RegionalParameterChannelPlanFileJson &value)
{
    std::cout << value.storage.toJsonString() << std::endl;
}

void doSmth(RegionalParameterChannelPlanFileJson &value)
{
    value.storage.setRegionalParametersVersion("1.0.3");
    RegionalParameterChannelPlan rb;
    rb.id = 14;
    rb.name = "RU864-870";  // 870MHz band
    rb.defaultRegion = true;
    rb.supportsExtraChannels = true;
    rb.bandDefaults.setValue(869100000, 0, 1, 2, 5, 6);
    rb.dataRates[0].setLora(BW_125KHZ, DRLORA_SF12);
    rb.dataRates[1].setLora(BW_125KHZ, DRLORA_SF11);
    rb.dataRates[2].setLora(BW_125KHZ, DRLORA_SF10);
    rb.dataRates[3].setLora(BW_125KHZ, DRLORA_SF9);
    rb.dataRates[4].setLora(BW_125KHZ, DRLORA_SF8);
    rb.dataRates[5].setLora(BW_125KHZ, DRLORA_SF7);
    rb.dataRates[6].setLora(BW_250KHZ, DRLORA_SF7);
    rb.dataRates[7].setFSK(5000);

    rb.maxPayloadSizePerDataRate[0].setValue(59, 51);
    rb.maxPayloadSizePerDataRate[1].setValue(59, 51);
    rb.maxPayloadSizePerDataRate[2].setValue(59, 51);
    rb.maxPayloadSizePerDataRate[3].setValue(123, 115);
    rb.maxPayloadSizePerDataRate[4].setValue(250, 242);
    rb.maxPayloadSizePerDataRate[5].setValue(250, 242);
    rb.maxPayloadSizePerDataRate[6].setValue(250, 242);
    rb.maxPayloadSizePerDataRate[7].setValue(250, 242);

    rb.maxPayloadSizePerDataRateRepeater[0].setValue(59, 51);
    rb.maxPayloadSizePerDataRateRepeater[1].setValue(59, 51);
    rb.maxPayloadSizePerDataRateRepeater[2].setValue(59, 51);
    rb.maxPayloadSizePerDataRateRepeater[3].setValue(123, 115);
    rb.maxPayloadSizePerDataRateRepeater[4].setValue(230, 222);
    rb.maxPayloadSizePerDataRateRepeater[5].setValue(230, 222);
    rb.maxPayloadSizePerDataRateRepeater[6].setValue(230, 222);
    rb.maxPayloadSizePerDataRateRepeater[7].setValue(230, 222);

    Channel ch;
    ch.setValue(868900000, 0, 5, true, false);
    rb.uplinkChannels.push_back(ch);
    rb.downlinkChannels.push_back(ch);
    ch.setValue(869100000, 0, 5, true, false);
    rb.uplinkChannels.push_back(ch);
    rb.downlinkChannels.push_back(ch);

    rb.setTxPowerOffsets(8, 0, -2, -4, -6, -8, -10, -12, -14);

    rb.setRx1DataRateOffsets(0, 6, 0, 0, 0, 0, 0, 0);
    rb.setRx1DataRateOffsets(1, 6, 1, 0, 0, 0, 0, 0);
    rb.setRx1DataRateOffsets(2, 6, 2, 1, 0, 0, 0, 0);
    rb.setRx1DataRateOffsets(3, 6, 3, 2, 1, 0, 0, 0);
    rb.setRx1DataRateOffsets(4, 6, 4, 3, 2, 1, 0, 0);
    rb.setRx1DataRateOffsets(5, 6, 5, 4, 3, 2, 1, 0);
    rb.setRx1DataRateOffsets(6, 6, 6, 5, 5, 4, 3, 2);
    rb.setRx1DataRateOffsets(7, 6, 7, 6, 5, 4, 3, 2);

    value.storage.bands.push_back(rb);
}

int main(int argc, char **argv) {

    config::rmFile(TEST_FN);

    RegionalParameterChannelPlanFileJson rbFile;

    std::cerr << "Load regional-parameters.json  file.." << std::endl;
    RegionalParameterChannelPlanFileJson *rbf = new RegionalParameterChannelPlanFileJson();
    rbf->init("regional-parameters.json", nullptr);
    printRegionBands(*rbf);
    delete rbf;

    std::cerr << "Load empty file.." << std::endl;
    // load empty file
    loadFile(rbFile, TEST_FN);
    std::cerr << "Print empty file.." << std::endl;
    printRegionBands(rbFile);

    std::cerr << "Change.." << std::endl;
    doSmth(rbFile);
    std::cerr << "Print changes.." << std::endl;
    printRegionBands(rbFile);

    std::cerr << "Save file.." << std::endl;
    saveFile(rbFile, TEST_FN);

    std::cerr << "Load saved file.." << std::endl;
    // load non-empty file
    loadFile(rbFile, TEST_FN);
    std::cerr << "Print saved file.." << std::endl;
    printRegionBands(rbFile);
}

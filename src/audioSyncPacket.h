#include <arduino.h>

// new "V2" audiosync struct - 40 Bytes
struct audioSyncPacket {
    char    header[6];      //  06 Bytes
    float   sampleRaw;      //  04 Bytes  - either "sampleRaw" or "rawSampleAgc" depending on soundAgc setting
    float   sampleSmth;     //  04 Bytes  - either "sampleAvg" or "sampleAgc" depending on soundAgc setting
    uint8_t samplePeak;     //  01 Bytes  - 0 no peak; >=1 peak detected. In future, this will also provide peak Magnitude
    uint8_t reserved1;      //  01 Bytes  - for future extensions - not used yet
    uint8_t fftResult[16];  //  16 Bytes
    float  FFT_Magnitude;   //  04 Bytes
    float  FFT_MajorPeak;   //  04 Bytes
};

static bool isValidUdpSyncVersion(const char *header) {
    return strncmp_P(header, "00002\0", 6) == 0;
}
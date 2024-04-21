#include <Arduino.h>
#include <hardware.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiMulti.h>
#include <audioSyncPacket.h>
#include "color.h"
#if __has_include("secret.h")
#include "secret.h" // make one yourself using the "secret_example.h"
#endif

WiFiMulti wifiMulti;
WiFiUDP udp;
audioSyncPacket lastPacket;

bool Button_R_Short   = false;
bool Button_L_Short   = false;
bool Button_L_Long    = false;

uint16_t ports[] = {11988, 11989}; // , 11990
uint8_t portIndex = 0;

void Init_Buttons()
{
  Button1.bind(Event_KeyPress,     [](){ Button_L_Short = true; });
  Button1.bind(Event_LongKeyPress, [](){ Button_L_Long  = true; });

  Button2.bind(Event_KeyPress,     [](){ Button_R_Short = true; });
  Button2.bind(Event_LongKeyPress, [](){ 
      Screen.fillScreen(TFT_BLACK);
      Screen.setTextSize(2);
      Screen.setTextColor(TFT_GREEN, TFT_BLACK);
      Screen.setTextDatum(MC_DATUM);
      int16_t h = Screen.fontHeight();
      Screen.drawString("Press again",  Screen.width() / 2, Screen.height() / 2 - h/2 );
      Screen.drawString("to wake up",   Screen.width() / 2, Screen.height() / 2 + h/2 );
      LightSleep(3000);
      DeepSleep(GPIO_NUM_35);
  });
}

void ShowSplashScreen(){
    // tft.setTextSize(1+random(4));
    Screen.setTextSize(5);
    Screen.setTextColor(TFT_WHITE, TFT_BLACK);
    Screen.setTextDatum(MC_DATUM);
    int16_t h = Screen.fontHeight();
    int16_t x = Screen.width() / 2;
    int16_t y = Screen.height() / 2;
    Screen.drawString("WLED SR" ,x,y-h);
    Screen.drawString("UDP",x,y  );
    Screen.drawString("Monitor",x,y+h);
}

bool Init_UDP(uint16_t port){
  Screen.setTextColor(TFT_SILVER);
  Screen.println(F("Connecting to UDP Multicast"));
  if(!udp.beginMulticast(IPAddress(239,0,0,1), port)) {
      Screen.setTextColor(TFT_RED);
      Screen.println(F("UDP connection failed"));
      return false;
  }
  Screen.setTextColor(TFT_GREEN);
  Screen.println(F("UDP connected"));
  return true;
}

bool CheckUDPPacket(){
  int packetSize = udp.parsePacket();
  if(packetSize == 0) return false; // no new packets
  // Serial.println(F("Incoming packet"));
  if(packetSize != sizeof(audioSyncPacket)){
     Serial.printf("Invalid Packet length: %d (should be %d)\n", packetSize, sizeof(audioSyncPacket));
     return false;
  }

  uint8_t buffer[packetSize];
  udp.read(buffer, packetSize);
 
  if (packetSize == sizeof(audioSyncPacket) && (isValidUdpSyncVersion((const char *)buffer))) {
    lastPacket = *reinterpret_cast<audioSyncPacket*>(buffer);

    //Serial.printf("  Header: %s\n", lastPacket->header);
    //bool anyNonZero = false;
    //for(uint8_t i=0;i<16 && !anyNonZero;i++)
    //  anyNonZero |= lastPacket.fftResult[i] > 0;
    //if(anyNonZero){
    //  Serial.print("  FFT: ");
    //  for(uint8_t i=0;i<16;i++)
    //    Serial.printf("%02X ", lastPacket.fftResult[i]);
    //  Serial.println();
    //}
    //Serial.println();    

    //DEBUGSR_PRINTLN("Finished parsing UDP Sync Packet v2");
    return true;
  }else{
    Screen.setTextColor(TFT_RED);
    Screen.println("Packet len: " + String(packetSize) + " " + String(sizeof(audioSyncPacket)) + " -> " + (isValidUdpSyncVersion((const char *)buffer)?"Valid":"INVALID"));
  }

  return false;
}

void showInfoScreen(){
  Button_R_Short = false;
  float vBatt = ReadBatteryVoltage();
  Screen.setTextSize(1);
  Screen.setCursor(0, 0);
  Screen.fillScreen(TFT_BLACK);
  Screen.setTextColor(TFT_GREEN, TFT_BLACK);
  Screen.println("Voltage: " + String(vBatt) + "V");
  Screen.printf("WiFi: %s (%d)\n", WiFi.SSID().c_str(), WiFi.RSSI());
  Screen.println("IP: " + WiFi.localIP().toString());
  while(!Button_R_Short) delay(100);
  Button_R_Short = false;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

#ifdef INIT_MULTIWIFI //check secret.h include
  initMultiWifi(wifiMulti);
#endif

  Init_Hardware();
  Init_Buttons();

  Screen.setRotation(3);

  Screen.fillScreen(TFT_BLACK);
  ShowSplashScreen();
  LightSleep(1000);

  Screen.setTextSize(1);
  Screen.setCursor(0, 0);
  Screen.fillScreen(TFT_BLACK);
  Screen.setTextColor(TFT_SILVER);

  Screen.println("Connecting to WiFi");

  if (wifiMulti.run() != WL_CONNECTED) {
      Screen.setTextColor(TFT_RED);
      Screen.println("WiFi connection Failed, restarting.");
      delay(3000);
      ESP.restart();
  }
  Screen.setTextColor(TFT_GREEN);
  Screen.println("Connected to WiFi: " + WiFi.SSID());
  Screen.println("IP: " + WiFi.localIP().toString());
  delay(1000);

  Screen.setTextColor(TFT_SILVER);
  Init_UDP(ports[portIndex]);
}

void ChangeUDPPort(){
  portIndex = (portIndex + 1) % (sizeof(ports)/sizeof(uint16_t));
  Screen.fillScreen(TFT_BLACK);
  Screen.setTextSize(1);
  Screen.setTextColor(TFT_SILVER);
  Screen.setCursor(0,0);
  Screen.println("Changing UDP port to:");
  Screen.setTextSize(2);
  Screen.setTextColor(TFT_GREEN);
  Screen.println(String(ports[portIndex]));
  Screen.setTextSize(1);
  Init_UDP(ports[portIndex]);
  delay(2000);
}

// 16 bit colorPicker : https://rgbcolorpicker.com/565
uint16_t colors[]={
  rainbowColor(  0), rainbowColor( 10), rainbowColor( 20), rainbowColor( 30), rainbowColor( 40), rainbowColor( 50), rainbowColor( 60), rainbowColor( 70),
  rainbowColor( 80), rainbowColor( 90), rainbowColor(100), rainbowColor(110), rainbowColor(120), rainbowColor(130), rainbowColor(140), rainbowColor(150)
};

bool firstPacket = true;
int rotation = 3;

void loop()
{
  if(Button_L_Short){
    Button_L_Short = false;
    rotation = (rotation + 1) % 4;
    Screen.setRotation(rotation);
    Screen.fillScreen(TFT_BLACK);
  }
  if(Button_L_Long){
    Button_L_Long = false;
    ChangeUDPPort();
    Screen.fillScreen(TFT_BLACK);
  }
  if(Button_R_Short){
    showInfoScreen();
    Screen.fillScreen(TFT_BLACK);
  }

  if(CheckUDPPacket()){
    if(firstPacket){
      Screen.fillScreen(TFT_BLACK);
      firstPacket = false;
    }
    int16_t scrWidth  = Screen.width();
    int16_t scrHeight = Screen.height();
    int16_t barWidth  = scrWidth / 16;
    uint16_t bgColor  = lastPacket.samplePeak > 0 ? 0x2945 : 0x0841; // Beat : Black
    for(uint8_t i=0; i<16; i++){
      int32_t barPos = map(lastPacket.fftResult[i], 0, 255, 3, scrHeight);
      Screen.fillRect(i * barWidth, 0               , barWidth-1, scrHeight-barPos-1, bgColor   );
      Screen.fillRect(i * barWidth, scrHeight-barPos, barWidth-1, barPos,             colors[i] );      
    }
  }
}

/*
  Make a copy as "secret.h" and edit accordingly.
  secret.h is in the .gitignore, so it won't end up in the source control!
*/
#pragma once
#define INIT_MULTIWIFI
#include <WiFiMulti.h>

void initMultiWifi(WiFiMulti& wifi){
  // add your init code here, like:
  wifi.addAP("SSID", "PSWD");
  wifi.addAP("SSID2", "PSWD2");
}
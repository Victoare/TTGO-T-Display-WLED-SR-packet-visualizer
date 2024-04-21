/**************************************************************
  Hardware setups, helpers
**************************************************************/
#pragma once

#ifndef TTGO_TDisplay_Hardware
#define TTGO_TDisplay_Hardware

#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <esp_adc_cal.h>
#include <driver/rtc_io.h>
#include <InterruptButton.h>

#define ADC_EN  14 // ADC_EN is the ADC detection enable port
#define ADC_PIN 34

#define BUTTON_1_GPIO 0
#define BUTTON_2_GPIO 35

InterruptButton Button1(BUTTON_1_GPIO, LOW);
InterruptButton Button2(BUTTON_2_GPIO, LOW);

int vref = 1100;
void Init_Hardware();
void HW_Init_Display();
void HW_Init_VoltageDetector();

TFT_eSPI Screen = TFT_eSPI();
//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void LightSleep(int ms);
void DeepSleep(gpio_num_t buttonToWakeUp);
float ReadBatteryVoltage();

/**************************************************************
  
**************************************************************/

void Init_Hardware() {
    HW_Init_Display();
    HW_Init_VoltageDetector();
}
    
void LightSleep(int ms){
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void DeepSleep(gpio_num_t buttonToWakeUp){
    int r = digitalRead(TFT_BL);
    digitalWrite(TFT_BL, !r);

    Screen.writecommand(TFT_DISPOFF);
    Screen.writecommand(TFT_SLPIN);
    //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    rtc_gpio_init(GPIO_NUM_14);
    rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_level(GPIO_NUM_14, 1);
    delay(500); 
    esp_sleep_enable_ext0_wakeup(buttonToWakeUp, 0);
    delay(200);
    esp_deep_sleep_start();   
}

float ReadBatteryVoltage(){
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    return battery_voltage;
}

void HW_Init_Display(){
  // Set up the display
  Screen.init();
}

void HW_Init_VoltageDetector(){
    /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */  
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV\n", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.println("Default Vref: 1100mV");
    }  
}

#endif
/*
*******************************************************************************
  Display of rotary encoder values and key status on the screen
*******************************************************************************
*/

#include <Arduino.h>
#include "M5AtomS3.h"
#include <M5GFX.h>
#include "Unit_Encoder.h"
#include "M5UnitHbridge.h"

M5GFX display;
M5Canvas canvas(&display);
Unit_Encoder sensor;
M5UnitHbridge driver;

void setup() {
    Wire.begin(2,1);
    auto cfg = M5.config();
    AtomS3.begin(cfg);
    sensor.begin();
    driver.begin(&Wire, HBRIDGE_I2C_ADDR, 2, 1, 100000L); //NOTE: Update Unit H-bridge.h for the correct i2c PINs

    AtomS3.Display.setTextColor(WHITE);
    AtomS3.Display.setTextSize(2);
    AtomS3.Display.clear();
    AtomS3.Display.drawString("1", 40, 5);
    AtomS3.Display.drawString("0", 40, 20);
}

signed short int last_value = 0;
signed short int last_btn = 0;

void loop() {
    signed short int encoder_value = sensor.getEncoderValue();
    bool btn_status                = sensor.getButtonStatus();
    if (last_value != encoder_value) {
        AtomS3.Display.clear();
        if (last_value > encoder_value) {
            sensor.setLEDColor(1, 0x000011);
        } else {
            sensor.setLEDColor(2, 0x111100);
        }
        last_value = encoder_value;
    } else {
        sensor.setLEDColor(0, 0x001100);
    }

    if (last_btn != btn_status) {
        last_btn = btn_status;
        AtomS3.Display.clear();
        AtomS3.Display.drawString(String(btn_status), 40, 5);
        AtomS3.Display.drawString(String(encoder_value), 40, 20);
    }

    if (!btn_status) {
        sensor.setLEDColor(0, 0xC800FF);
    }
    
    AtomS3.Display.drawString(String(btn_status), 40, 5);
    AtomS3.Display.drawString(String(encoder_value), 40, 20);
    delay(20);
}
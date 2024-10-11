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

uint64_t forsink = 100; //delay between runs
uint64_t tempus;
int ontime = 30; // how long time to run motor
bool progmode = true; //programming mode or not
bool newpress = true; // monitor if button just pressed 
int mstatus = 0; // defines which state the system is in

signed short int last_value = 0;
signed short int last_btn = 0;

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
    AtomS3.Display.setTextSize(3);
    AtomS3.Display.clear();
    //AtomS3.Display.drawString("1", 40, 5);
    //AtomS3.Display.drawString("0", 40, 20);
    tempus = millis();
}

void loop() {

    AtomS3.update();
    if (AtomS3.BtnA.wasPressed()) {
        mstatus = mstatus +1;
        if(mstatus == 5) mstatus = 0; // go back to base state
        //progmode = !progmode;
        AtomS3.Display.clear();
        AtomS3.Display.drawString(String(mstatus), 10, 100);
        newpress = true;
    }

        switch (mstatus) {

        case 0: //run motor
        { 
            if (newpress) {
                AtomS3.Display.drawString("Running", 5, 0);
                newpress = false;
            }

            if (millis() - tempus >= forsink) // to be set by adjustment (100)
            {
                AtomS3.Display.drawString("Running", 5, 0);
                AtomS3.Display.drawString(String(ontime), 10, 30);
                AtomS3.Display.drawString(String(forsink), 10, 60);   
                driver.setDriverDirection(HBRIDGE_FORWARD); // Set peristaltic pump in forward to take out BR content
                //driver.setDriverDirection(HBRIDGE_BACKWARD)
                driver.setDriverSpeed8Bits(127); //Run pump in half speed
                delay(ontime); // to be set by adjustment (30)
                driver.setDriverDirection(HBRIDGE_STOP);
                driver.setDriverSpeed8Bits(0);  //Stop pump
                tempus = millis();
            }
            
            break;
        }

        case 1: // read encoder for ON time in ms
        {
            signed short int encoder_value = sensor.getEncoderValue();
            bool btn_status                = sensor.getButtonStatus();
            ontime = encoder_value;

            if (newpress) {
                AtomS3.Display.drawString("On time", 5, 0);
                AtomS3.Display.drawString(String(encoder_value), 10, 30);
                newpress = false;
            }

            if (last_value != encoder_value) {
                AtomS3.Display.setTextColor(BLACK);
                AtomS3.Display.drawString(String(last_value), 10, 30);
                AtomS3.Display.setTextColor(WHITE);
                AtomS3.Display.drawString(String(encoder_value), 10, 30);
                last_value = encoder_value;
            }

                //if (last_value > encoder_value) {
                //    sensor.setLEDColor(1, 0x000011); // change color on LED1 when forward
                //} else {
                //    sensor.setLEDColor(2, 0x111100); // color on LED2 when backward
                //}

            //} else {
            //    sensor.setLEDColor(0, 0x001100); // both LEDs are low intensity when not pressed
            //}

            if (last_btn != btn_status) {
                last_btn = btn_status;
            //    AtomS3.Display.clear();
            //    AtomS3.Display.drawString(String(btn_status), 40, 5);
            //    AtomS3.Display.drawString(String(encoder_value), 40, 20);
            }

            //if (!btn_status) {
            //    sensor.setLEDColor(0, 0xC800FF); //light up both LEDs when pressed
            //}
;
            delay(20);
            break;
        }

        case 2: // read encoder for ON time in ms
        {
            signed short int encoder_value = sensor.getEncoderValue();
            forsink = encoder_value * 100;

            if (newpress) {
                AtomS3.Display.drawString("Delay", 5, 0);
                AtomS3.Display.drawString(String(encoder_value*100), 10, 60);
                newpress = false;
            }

            if (last_value != encoder_value) {
                AtomS3.Display.setTextColor(BLACK);
                AtomS3.Display.drawString(String(last_value*100), 10, 60);
                AtomS3.Display.setTextColor(WHITE);
                AtomS3.Display.drawString(String(encoder_value*100), 10, 60);
                last_value = encoder_value;
            }

            break;    
        }

        }
}
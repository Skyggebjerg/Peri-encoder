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
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 2

uint64_t forsink = 1000; //delay between runs
int save_forsink = 10; // forsink saved in EEPROM as int (forsink divided by 100)
uint64_t tempus;
int ontime = 30; // how long time to run motor
bool newpress = true; // monitor if button just pressed 
int mstatus = 0; // defines which state the system is in

signed short int last_value = 0;
signed short int last_btn = 1;

M5GFX display;
M5Canvas canvas(&display);
Unit_Encoder sensor;
M5UnitHbridge driver;

void setup() {
    // initialize EEPROM with predefined size
    EEPROM.begin(EEPROM_SIZE);
    ontime = EEPROM.read(0);
    save_forsink = EEPROM.read(1);
    forsink = save_forsink*100;
    Wire.begin(2,1);
    auto cfg = M5.config();
    AtomS3.begin(cfg);
    sensor.begin();
    driver.begin(&Wire, HBRIDGE_I2C_ADDR, 2, 1, 100000L); //NOTE: Update Unit H-bridge.h for the correct i2c PINs

    AtomS3.Display.setTextColor(WHITE);
    AtomS3.Display.setTextSize(3);
    AtomS3.Display.clear();
    tempus = millis();
}

void loop() {

    //AtomS3.update();
    bool btn_status                = sensor.getButtonStatus();
    //if (AtomS3.BtnA.wasPressed()) { // Change mode when click ATOMS3 button
    if (last_btn != btn_status) { // Change mode when click encoder - check if encoder is pressed down or up
        if (!btn_status) { // Only change mode when encoder go from high to low
            mstatus = mstatus +1;
            if(mstatus == 5) mstatus = 0; // go back to base state
            AtomS3.Display.clear();
            AtomS3.Display.drawString(String(mstatus), 10, 100);
            newpress = true;
        }
        last_btn = btn_status;
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
        } // end of case 0

        case 1: // read encoder for ON time in ms
        {
            signed short int encoder_value = sensor.getEncoderValue();
            //ontime = encoder_value;

            if (newpress) {
                AtomS3.Display.drawString("On time", 5, 0);
                AtomS3.Display.drawString(String(ontime), 10, 30);
                last_value = encoder_value; // Update the last value
                newpress = false;
            }

            if (last_value != encoder_value) {
                int relative_change = encoder_value - last_value; // Calculate the relative change

                AtomS3.Display.setTextColor(BLACK);
                AtomS3.Display.drawString(String(ontime), 10, 30); // Clear the previous value
                AtomS3.Display.setTextColor(WHITE);
                ontime = ontime + relative_change; // Update the value
                AtomS3.Display.drawString(String(ontime), 10, 30); // Display the updated change

                last_value = encoder_value; // Update the last value
            }
            delay(20);
            break;
        } // end of case 1

        case 2: // read encoder for forsink in ms
        {
            signed short int encoder_value = sensor.getEncoderValue();
            //forsink = encoder_value * 100;

            if (newpress) {
                AtomS3.Display.drawString("Delay", 5, 0);
                AtomS3.Display.drawString(String(forsink), 10, 60);
                last_value = encoder_value; // Update the last value
                newpress = false;
            }

            if (last_value != encoder_value) {
                int relative_change = encoder_value - last_value; // Calculate the relative change
                AtomS3.Display.setTextColor(BLACK);
                AtomS3.Display.drawString(String(forsink), 10, 60);
                AtomS3.Display.setTextColor(WHITE);
                forsink = forsink + relative_change*100; // Update the value
                AtomS3.Display.drawString(String(forsink), 10, 60);
                last_value = encoder_value;
            }
            break;    
        } // end of case 2

        case 3: // check if we want to save ontime and forsink by pres Atom button
        {
            if (newpress) {
                AtomS3.Display.drawString("Save ?", 5, 0);
                AtomS3.Display.drawString(String(ontime), 10, 30);
                AtomS3.Display.drawString(String(forsink), 10, 60);
                newpress = false;
            }
            
            AtomS3.update();
            if (AtomS3.BtnA.wasPressed()) { // Save to EEPROM
                EEPROM.write(0, ontime);
                save_forsink = forsink/100;
                EEPROM.write(1,save_forsink);
                EEPROM.commit();

                // Flash the display in black and green
                for (int i = 0; i < 3; i++) {
                    AtomS3.Display.fillScreen(TFT_BLACK);
                    delay(200);
                    AtomS3.Display.fillScreen(TFT_GREEN);
                    delay(200);
                }

                // Display "Saved" with a black background
                AtomS3.Display.fillScreen(TFT_BLACK);
                AtomS3.Display.setTextColor(TFT_WHITE, TFT_BLACK); // White text on black background
                AtomS3.Display.drawString(String(ontime), 10, 30);
                AtomS3.Display.drawString(String(forsink), 10, 60);
                AtomS3.Display.drawString("Saved", 30, 100);

            }
            break;    
        } // end of case 3

    

    } // end of switch cases
}
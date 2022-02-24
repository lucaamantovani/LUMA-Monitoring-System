#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ArduinoBLE.h>
#include "Logo.h"

#define DISPLAY_Address 0x3C
#define UpperThreshold 650
#define LowerThreshold 530

Adafruit_SSD1306 display(128, 64);
BLEService ecgService("57a806f7-ab55-4932-ada2-5f2c5a94b3f7");
BLEByteCharacteristic ecgCharacteristic("57a806f7-ab55-4932-ada2-5f2c5a94b3f7", BLERead);

int a=0;
int lasta=0;
int lastb=0;
int LastTime=0;
int ThisTime;
int BPM=0;

bool BPMTiming=false;
bool BeatComplete=false;

void setup() {

  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_Address);
  display.clearDisplay();
  display.drawBitmap(0, 0, Luma_Logo, 128, 64, WHITE);
  display.display();

  pinMode(9, INPUT);
  pinMode(8, INPUT);

  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);

  if (!BLE.begin())
    for(;;);

  BLE.setLocalName("LUMA Monitoring System");
  
  BLE.setAdvertisedService(ecgService);
  
  ecgService.addCharacteristic(ecgCharacteristic);

  BLE.addService(ecgService);

  ecgCharacteristic.setValue(0);

  BLE.advertise();
  
  delay(2000);
  display.clearDisplay();
  display.drawBitmap(0, 0, BLE_enabled, 128, 64, WHITE); 
  display.display();

}

void loop() {

  BLEDevice central = BLE.central();
  
  if (central) {
    
    display.clearDisplay();
    display.drawBitmap(0, 0, BLE_connected, 128, 64, WHITE);
    display.display();

    display.clearDisplay();
    
    while (central.connected()) {
 
      int value = 0;
    
      if ((digitalRead(8) != 1) && (digitalRead(9) != 1))
      {
        
        if(a > 127)
        {
          display.clearDisplay();
          a = 0;
          lasta = a; 
        }
        
        ThisTime = millis();
        int value = analogRead(0);
        display.setTextColor(WHITE);
        int b = 60-(value/16);
        display.writeLine(lasta,lastb,a,b,WHITE);
        lastb = b;
        lasta = a;
        
        if (value>UpperThreshold)
        {
          
          if (BeatComplete)
          {  
            BPM=ThisTime-LastTime;
            BPM=int(60/(float(BPM)/1000));
            BPMTiming=false;
            BeatComplete=false;
            tone(8,1000,250);
          }
          
          if(BPMTiming==false)
          {
            LastTime=millis();
            BPMTiming=true;
          }
          
        }
        
        if ((value<LowerThreshold)&(BPMTiming)) 
          BeatComplete=true;

        if(BPM>100 || BPM<45)
        {
          digitalWrite(3, HIGH);
          digitalWrite(4, LOW);
        }else{ 
          digitalWrite(4, HIGH);
          digitalWrite(3, LOW); 
        }
        
        ecgCharacteristic.writeValue(BPM);
        display.writeFillRect(0,50,128,16,BLACK);
        display.setCursor(0,50);
        display.print("BPM:");
        display.print(BPM);
        display.setCursor(50,50);
        display.print("LUMA IoT MS");
        display.display();
        a++;
      }
      delay(1);  
    }
    
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    display.clearDisplay(); //for Clearing the display
    display.drawBitmap(0, 0, Warning, 128, 64, WHITE);
    display.display();  
  }
  
}

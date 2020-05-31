/*
  433 MHz RF Module Transmitter Demonstration 2
  RF-Xmit-Demo-2.ino
  Demonstrates 433 MHz RF Transmitter Module with DHT-22 Sensor
  Use with Receiver Demonstration 2
  DroneBot Workshop 2018
  https://dronebotworkshop.com
*/

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

//LCD display 
#include <LiquidCrystal.h>

// Include DHT Libraries from Adafruit
#include "DHT.h";

//Built in LED on Arduino nano
byte ledPin = 13; 

// LCD Monitors 
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// RF Transmitter and receiver 
RH_ASK rf_driver; //Transmit pin is 12. Receive pin is 10 by default on Arduino Nano

//TimeDelay for RF transmission
long lastTransmit= 0; 

//Listing of all slave units that are communicating with Master
String slave1ID = "Workshop";
String slave2ID = "Bathroom";


//Timeout management for RF communication
long timeoutInterval = 10000;

long brTimeoutTime;
long wsTimeoutTime;
byte brTimeoutPin = 9;
byte wsTimeoutPin = 8 ;

void setup() {
  pinMode(ledPin,OUTPUT);
  pinMode(rs,OUTPUT);
  pinMode(brTimeoutPin,OUTPUT);
  pinMode(wsTimeoutPin,OUTPUT);

  
  // Initialize ASK Object (RF receiver and transmitter)
  rf_driver.init();

  // Initialize serial commnunication for debugging 
  Serial.begin(9600);

   // Initialize LCD display unit 
   lcd.begin(16, 2);
   lcd.setCursor(0,0);              
   lcd.print("2 way muti-C RF");
   lcd.setCursor(0,1);
   lcd.print("Master unit");
   delay(2000);
}

void transmit(String slaveID) {
      Serial.println("Sending message to: "+ slaveID);
      digitalWrite(ledPin,HIGH);

      //For now, only STATUS command exist. Can be extended. 
      String str_out = slaveID + "%" + "STATUS"+"%"; // Send command to the proper 

      //Converting str_out in an array of characters for transmission.
      const char *msg = str_out.c_str();
    
      rf_driver.send((uint8_t *)msg, strlen(msg));
      rf_driver.waitPacketSent();
    
      //Check for the slave response for 3 seconds.
      // If it doesn't work, timeout. 
      long timeOut = millis()+3000;
      int msgReceived = -1; 
      while (msgReceived ==-1){
         msgReceived = checkReceive();
         if (millis()>timeOut) {
          return;
         }
         
      }
      digitalWrite(ledPin,LOW);
    }
    

int checkReceive(){
  //Check if there is a received message in the RF buffer. If so, interpret the message. 
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    if (rf_driver.recv(buf, &buflen)) // Non-blocking
    {
        
        // Message with a good checksum received, dump it.
        
        Serial.println((char*)buf); 

        //Interpretation
        int sepparatorIndex = 0; 
        String cmdID; 
        String cmd; 
        String tString;
        String hString; 
        for (int i = 0; i < RH_ASK_MAX_MESSAGE_LEN; i++) {
          if(buf[i] =='%') {//Anytime there is a % charactor, it needs to go
            //to the next data information. 
            sepparatorIndex ++; 
          }
          else {
            if (sepparatorIndex ==0) {
               cmdID = cmdID+(char)buf[i];
            }
            else if (sepparatorIndex ==1) {
              cmd = cmd + (char)buf[i];
            }
            else if (sepparatorIndex ==2) {
              hString = hString +(char) buf[i];
              
            }
            else if (sepparatorIndex == 3) {
              tString = tString + (char) buf[i];
            }
          }  
        }
        
        Serial.println("T = " + tString); 
        Serial.println("H = " + hString);

        String line; 
        if (cmdID == "Workshop"){
          lcd.setCursor(0,0); 
          line = "WS T"+tString+"C "+"H"+hString+"%         ";
          wsTimeoutTime = millis() + timeoutInterval; 
        }
        if (cmdID == "Bathroom") {
          lcd.setCursor(0,1); 
          line = "BR T"+tString+"C "+"H"+hString+"%         ";
          brTimeoutTime = millis() + timeoutInterval;
        }
        
        lcd.print(line); //Print the humidity and temperature on the 
        //corresponding line    
        return 1; 
    }
    else {
      return -1;
    }
}

void checkForTimeout() {
  // If there is no communication for 10 seconds, status light becomes low.
  if (millis() > brTimeoutTime) {
    digitalWrite(brTimeoutPin,LOW);
  }
  else {
    digitalWrite(brTimeoutPin,HIGH);
  }
  // If there is no communication for 10 seconds, status light becomes low.
  if (millis() > wsTimeoutTime) {
    digitalWrite(wsTimeoutPin,LOW);
  }
  else {
    digitalWrite(wsTimeoutPin,HIGH);
  }
}

void loop()
{
   transmit(slave1ID); // Send first message and waid for answer
   transmit(slave2ID); // Send second message and wait for answer 
   checkForTimeout(); // After 10 seconds without answer, turn off status LED light
}

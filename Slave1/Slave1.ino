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

// CHANGE THIS ID SO IT'S UNIQUE FOR EACH SLAVES OF THE COMMUNICATION NETWORK
String slaveID = "Workshop"; 

// Include DHT Libraries from Adafruit
// Dependant upon Adafruit_Sensors Library
#include "DHT.h";

// Define Constants
#define DHTPIN 7       // DHT-22 Output Pin connection
#define DHTTYPE DHT11   // DHT Type is DHT 22 (AM2302)

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

// Define Variables
int hum;    // Stores humidity value in percent
int temp;   // Stores temperature value in Celcius
// Define output strings
String str_humid;
String str_temp;
String str_out;

// RF transmitter and receiver 
RH_ASK rf_driver; //Receive pin is 10 by default on Arduino Nano
byte ledPin = 3; // Communication indicator pin 

//Timing Delay between reception of a command and sending an answer 

//long transmitInterval = 200; 

void setup() {
  pinMode(ledPin,OUTPUT); // Communication indicator pin 
  
  Serial.begin(9600);

   // Initialize ASK Object (RF receiver and transmitter)
  if (!rf_driver.init()) { //Transmit pin is 12. Receive pin is 10 by default on Arduino Nano
    Serial.println("init failed");
  }
  else {
    Serial.println("rf init successfull");
  }
  
  // Start DHT Sensor
  dht.begin();
}

void transmit(String message) {
    
      digitalWrite(ledPin,HIGH);//Communication status 
      
      const char *msg = message.c_str();//Convert string message into a 
      //Character array
    
      rf_driver.send((uint8_t *)msg, strlen(msg));
      rf_driver.waitPacketSent();
    
      digitalWrite(ledPin,LOW);
}
void checkReceive(){//Check for incoming message. 
  //If there is a message, check if the message is adressed to the unit.
  //If so, react to it. 
  
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  if (rf_driver.recv(buf, &buflen)) // Non-blocking
    {
        
        String cmdID; 
        String cmd; 
        int sepparatorIndex = 0; 

        //Message interpretation
        for (int i = 0; i < RH_ASK_MAX_MESSAGE_LEN; i++) {
          if(buf[i] =='%') {
            sepparatorIndex ++; 
          }
          else {
            if (sepparatorIndex ==0) {
               cmdID = cmdID+(char)buf[i];
            }
            else if (sepparatorIndex ==1) {
              cmd = cmd + (char)buf[i];
            }
          }  
        }
        Serial.println(cmdID);
        Serial.println(cmd);
        interpretCommand(cmdID,cmd);
    }
}

void interpretCommand(String cmdID, String cmd) {
  if (cmdID == slaveID) {//ONLY react to the message if it was 
    //adressed to this unit 
    if (cmd =="STATUS") {
        hum = dht.readHumidity();  // Get Humidity value
        temp= dht.readTemperature();  // Get Temperature value
    
        // Convert Humidity to string
        str_humid = String(hum);
    
        // Convert Temperature to string
        str_temp = String(temp);

        String transmitMessage = cmdID+'%'+cmd+"%"+str_humid+"%"+str_temp+"%";
        Serial.println("Transmit message = " + transmitMessage);
        delay(200); // Small delay before transmitting the answer, to 
        //Make sure the RF transmitter is ready for reception. 
        transmit(transmitMessage);
    }
  }
  
}
void loop()
{
   checkReceive();
}

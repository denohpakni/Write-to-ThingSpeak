#include <Arduino.h>
#include <dht11.h>
#include <SPI.h>  // Communication lib
#include <WiFiNINA.h>  // wifi lib
#include "mySecrets.h"  // contains my secret items like wifi password
#include <ThingSpeak.h>

// Hardware info
#define SENSOR_POWER 13                            // Connect the power for the soil sensor here.
#define DHT11PIN 9
dht11 DHT11;

// Soil Moisture variables
const int dry = 460;
const int wet = 239;

// Network information.
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;  // your network key Index number (needed only for WEP)
WiFiClient  client;

// ThingSpeak information.
unsigned long myChannelNumber = SECRET_CH_ID;                
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Global Variables
// Initialize our values
int Humidity = 0;
int Temp = 0;
int soilMoistureValue = 0;
String myStatus = "";

// Connect to the local Wi-Fi network
int connectWifi()
{
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin( SECRET_SSID , SECRET_PASS );
        Serial.println( "Connecting to Wi-Fi" );
        delay(2500);
    }
    Serial.println( "we are Connected!" );  // Inform the serial monitor.
}

void setup() {
  Serial.begin(9600);
  connectWifi();
  Serial.print("Loading Data bwana....");
  Serial.println("DONE!");
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  
  Serial.println();
  int chk = DHT11.read(DHT11PIN);
  Humidity = (float)DHT11.humidity, 2;
  Serial.print("Humidity (%): ");
  Serial.println(Humidity);

  Temp = (float)DHT11.temperature, 2;
  Serial.print("Temperature (C): ");
  Serial.println(Temp);  
  
  int sensorVal = analogRead(A0);
  // Sensor has a range of 239 to 595
  // We want to translate this to a scale or 0% to 100%
  int soilMoistureValue = map(sensorVal,wet,dry,100,0); 
  Serial.print("SoilMoisture is ");
  Serial.print(soilMoistureValue);
  Serial.println(" %"); 

  /* Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
     pieces of information in a channel.  
  */

  // set the fields with the values
  ThingSpeak.setField(1, Temp);
  ThingSpeak.setField(2, Humidity);
  ThingSpeak.setField(3, soilMoistureValue);

// figure out the status message
  if(Temp > Humidity){
    myStatus = String("Temp is greater than Humidity"); 
  }
  else if(Temp < soilMoistureValue){
    myStatus = String("Temp is less than soilMoistureValue");
  }
  else{
    myStatus = String("Temp equals Humidity");
  }
  
  // set the status
  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

   delay(15000);  // 15 seconds gap is good to alllow proper reading and posting
}

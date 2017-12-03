// Meteocuadros
// JoseM <jose.nesga@gmail.com> - 2017
// v 0.01 - Initial Release
//
// This code is licensed under a Creative Commons Attribution Share-Alike license. 
// - Arduino NANO
// - BMP180 
// - DHT22
// - FC-37
// - SIM800L
//

#include <SoftwareSerial.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include "DHT.h"


//Pinout
SoftwareSerial Serial_SIM800L(10, 11); // RX, TX for SIM800L
int PIN_led_green = 3;    // D3
int PIN_led_orange = 2;   // D2
int PIN_rain_analog = 1;  // A1
const int PIN_DHT = 12;     // D12


//Read values from sensors
String timestamp;       //GSM-Date & Time (YYMMDDHHmmss)
int rainA;              //Rain Sensor value (analog value 0-1024)
double Temperature;     //Temperature from bmp180 sensor
double Fixed_Pressure;  //Pressure from bmp180 sensor. Fixed to local Altitude
float Humidity;         //Humidity from DHT sensor
float Temperature2;     //Temperature fromo DHT sensor


//GMP180 Sensor (Temp & Press)
SFE_BMP180 bmp180;      
#define ALTITUDE 837.0   //LEON (Meters)
char BMP180_wait_time;   //Time to wait for BMP180 query results

//DHT
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(PIN_DHT, DHTTYPE);

//Other vars
int loopNumber;        // loop() function execution counter



/*********************************************************************************/
void setup()  
{

  // Open Serial Port for Debug
  Serial.begin(9600);
  Serial.println("Running Setup()...");  
  
  // PinMode LEDs
  pinMode(PIN_led_green, OUTPUT);     
  pinMode(PIN_led_orange, OUTPUT); 
    
  // Flash both sensors while setup
  digitalWrite(PIN_led_green, HIGH);
  digitalWrite(PIN_led_orange, HIGH);
  delay(250);
  digitalWrite(PIN_led_green, LOW);
  digitalWrite(PIN_led_orange, LOW);
  delay(250);
  digitalWrite(PIN_led_green, HIGH);
  digitalWrite(PIN_led_orange, HIGH);
  delay(250);
  digitalWrite(PIN_led_green, LOW);
  digitalWrite(PIN_led_orange, LOW);
  delay(250); 
  digitalWrite(PIN_led_green, HIGH);
  digitalWrite(PIN_led_orange, HIGH);
  delay(250);
    digitalWrite(PIN_led_green, LOW);
  digitalWrite(PIN_led_orange, LOW);
  delay(250);  
  digitalWrite(PIN_led_green, HIGH);
  digitalWrite(PIN_led_orange, HIGH);

  // PinMode Rain Sensor 
  pinMode(PIN_rain_analog, INPUT);
    
  // Open Software-Serial Por for SIM800L Com
  Serial_SIM800L.begin(9600);
  
  //GPRS Configuration
  delay(30000);                                                      //Time to wait for get GSM Signal
  Serial_SIM800L.println("AT+SAPBR=3,1,\"APN\",\"orangeworld\"");    //Configure GPRS APN
  Serial_SIM800L.flush();
  delay(2000);

  //DHT Start
  dht.begin();

  //bmp180 Start
  bmp180.begin();

  loopNumber = 0;
  
  //LEDs (both) sequency during setup is: 
  //   1. Blink 3 times
  //   2. Stay ON while sensor setup
  //   3. Stay OFF until loop() starts
  digitalWrite(PIN_led_green, LOW);
  digitalWrite(PIN_led_orange, LOW);
  delay(2000);    // Wait a few seconds before start looping

}




/*********************************************************************************/
void loop()
{

  Serial.print("Running loop( ");
  Serial.print(loopNumber);
  Serial.println(")...");
  
  //LEDs sequency during loop is:
  //    1.  Green ON while reading sensor data
  //    2.  Green OFF when data collected
  //    3.  Orange ON while sending data over GPRS
  //    4.  Orangle OFF when data is sent
  //    5.  BOTH ON while sleeping to the next measure
  //    6.  BOTH OFF before next loop
  digitalWrite(PIN_led_green, HIGH);

  //Getting DateTime from GPS Network
  while(Serial_SIM800L.available() > 0) { Serial_SIM800L.read(); }      // Clean buffer
  Serial_SIM800L.write("AT+CCLK?\r\n");                                 // ASK for Date and Time
  Serial_SIM800L.flush();                                               // Wait until Serial is flushed
  delay(2500);                                                          // Security wait before reading answer
  char SIM800_CCLK_Data[50] = "";                                       // Answer... 50 characters are enough 
  if (loopNumber == 0) { 
    loopNumber++;
    return;                                                             // In the first loop, do nothing. This is a hack to avoid getting no date and time the first loop. Not very sure why it happens :(
  }
  for (int i=0; i<=50; i++) {
      SIM800_CCLK_Data[i] = Serial_SIM800L.read();                      // Read the answer
  }
  char datetime[] = { SIM800_CCLK_Data[19], \  
                      SIM800_CCLK_Data[20], \       
                      SIM800_CCLK_Data[22], \
                      SIM800_CCLK_Data[23], \
                      SIM800_CCLK_Data[25], \
                      SIM800_CCLK_Data[26], \
                      SIM800_CCLK_Data[28], \
                      SIM800_CCLK_Data[29], \ 
                      SIM800_CCLK_Data[31], \
                      SIM800_CCLK_Data[32], \
                      SIM800_CCLK_Data[34], \
                      SIM800_CCLK_Data[35], \
                     '\0'  };
  timestamp = datetime;                                                // <----- USABLE VALUE     


  //Getting Analog Rain Sensor Data: 0 (Closed Contact) & 1024 (Open Contact)
  rainA = analogRead(PIN_rain_analog);                                // <----- USABLE VALUE


  //Getting Temperature and Pressure from BMP180. Temperature is neccesary to fix Pressure
  BMP180_wait_time = bmp180.startTemperature();                //This functions returns time in ms you have to wait to receive de data
  delay(BMP180_wait_time);                                     //So you wait that time
  BMP180_wait_time = bmp180.getTemperature(Temperature);       //And then you grab the data

  BMP180_wait_time = bmp180.startPressure(3);
  delay(BMP180_wait_time);
  BMP180_wait_time = bmp180.getPressure(Fixed_Pressure,Temperature);
  Fixed_Pressure = bmp180.sealevel(Fixed_Pressure,ALTITUDE);  //Fix to the local altitude

  //Convert to Strings
  char* Fixed_Pressure_Str_TMP = "";
  dtostrf(Fixed_Pressure,4,0,Fixed_Pressure_Str_TMP);
  String VALUE_Fixed_Pressure = Fixed_Pressure_Str_TMP;                // <----- USABLE VALUE 


  //Getting Humidity and Temperature from DHT
  Humidity = dht.readHumidity();
  Temperature2 = dht.readTemperature();

  //Convert to String
  char* Temperature2_Str_TMP = "";
  dtostrf(Temperature2,2,1,Temperature2_Str_TMP);
  String VALUE_Temperature2 = Temperature2_Str_TMP;                    // <----- USABLE VALUE 

  char* Humidity_Str_TMP = "";
  dtostrf(Humidity,2,0,Humidity_Str_TMP);
  String VALUE_Humidity = Humidity_Str_TMP;                            // <----- USABLE VALUE
  Serial.println(" Collected Data");
  Serial.print(" Datetime: ");
  Serial.println(timestamp);
  Serial.print(" Humidity %: ");
  Serial.println(VALUE_Humidity);
  Serial.print(" Temperature2: ");
  Serial.println(VALUE_Temperature2);
  Serial.print(" Pressure: ");
  Serial.println(VALUE_Fixed_Pressure);
  Serial.print(" RainA: ");
  Serial.println(rainA);

  digitalWrite(PIN_led_green, LOW);
  digitalWrite(PIN_led_orange, HIGH);


  //Generate HTTP Query
  // KEY=ASDF&DATETIME=201612290833&TEMPERATURAAMBIENTE=12.3&HUMEDADRELATIVA=23&PRESIONATMOSFERICA=916&DETECCIONLLUVIA=0&PLUVIOMETROTICSACTUAL=12&PLUVIOMETROTICSANTERIOR=122

  String HTTPPARA_LINE = "AT+HTTPPARA=\"URL\",\"https://www.whatever.com:443/insert.php?KEY=XxXXXxxXXXxXxxXXXxxxx";     // REMEMBER CHANGING THE >>>>> KEY <<<<<<    

  HTTPPARA_LINE.concat("&DATETIME=");
  HTTPPARA_LINE.concat(timestamp);

  HTTPPARA_LINE.concat("&DETECCIONLLUVIA=");
  HTTPPARA_LINE.concat(rainA);

  HTTPPARA_LINE.concat("&TEMPERATURAAMBIENTE=");
  HTTPPARA_LINE.concat(VALUE_Temperature2);

  HTTPPARA_LINE.concat("&HUMEDADRELATIVA=");
  HTTPPARA_LINE.concat(VALUE_Humidity);

  HTTPPARA_LINE.concat("&PRESIONATMOSFERICA=");
  HTTPPARA_LINE.concat(VALUE_Fixed_Pressure);

  HTTPPARA_LINE.concat("\"");

  Serial.print(" HTTP Query: ");
  Serial.println(HTTPPARA_LINE);  

  Serial_SIM800L.println("AT+SAPBR=1,1");            // Wake Up Neo...
  delay(5000);
  Serial_SIM800L.println("AT+HTTPINIT");             // HTTP API Start
  delay(2000);
  Serial_SIM800L.println(HTTPPARA_LINE);             // This is the URL
  delay(1000);
  Serial_SIM800L.println("AT+HTTPSSL=1");            // Enable HTTPS
  delay(1000);
  //Serial_SIM800L.println("AT+HTTPSSL?");         // GET
  //delay(5000);
  Serial_SIM800L.println("AT+HTTPACTION=0");         // GET
  delay(10000);
  Serial_SIM800L.println("AT+HTTPTERM");             // HTTP API End
  delay(1000);
  Serial_SIM800L.println("AT+SAPBR=0,1");            // Go to sleep again (energy save)
  Serial_SIM800L.flush();                            // Wait until Serial is flushed

  digitalWrite(PIN_led_orange, LOW);
  Serial.println("End Loop. Sleeping...");
  Serial.println();

  // Sleep for i second blinking both LEDs alternative
  for (int i=0; i<600; i++){
    digitalWrite(PIN_led_green, HIGH);
    digitalWrite(PIN_led_orange, LOW);
    delay(500);
    digitalWrite(PIN_led_green, LOW);
    digitalWrite(PIN_led_orange, HIGH);
    delay (500);
  }
  digitalWrite(PIN_led_green, LOW);
  digitalWrite(PIN_led_orange, LOW);
  
  loopNumber++;
}


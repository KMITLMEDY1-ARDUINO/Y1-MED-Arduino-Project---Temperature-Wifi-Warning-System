#include "ESP8266WiFi.h"
#include "Gsender.h"
#include "DallasTemperature.h"
#include "OneWire.h"
  
#pragma region Globals
// Data wire is plugged into pin D3 on the Arduino 
#define ONE_WIRE_BUS 3
double Temperature;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
//OneWire oneWire(ONE_WIRE_BUS);  
  OneWire oneWireBus_3(D3);

// Pass oneWire reference to Dallas Temperature 
DallasTemperature ds18b20_3(&oneWireBus_3);
double fnc_ds128b20_temp(int _index)
{
 ds18b20_3.requestTemperatures();
  return ds18b20_3.getTempCByIndex(_index);
}                       

    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "Patient Temperature Monitor!";

#pragma region Globals
const char* ssid = "Paul";                           // WIFI network name
const char* password = "unontakarn";                 // WIFI network password
uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void setup(void)

{
  // start serial port for temperature probe
  // Serial.begin(9600); 
  // Start up the temp probe
  //ds18b20_3.begin();
  // start serial port for WiFi module

    Serial.begin(115200);
    ds18b20_3.begin();
    connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

    //Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    //String subject = "Patient Temperature Monitor!";
    if(gsender->Subject(subject)->Send("y1kmitlmed.arduino@gmail.com", "Patient Alert")) {
        Serial.println("Patient Temp Alert");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}
    
void loop(void)

{
  

  // Get temperature reading
  ds18b20_3.requestTemperatures();
  // Get temperature reading in Celsius from first IC on wire
  const float tempC = ds18b20_3.getTempCByIndex(0);
 char buff[100];
 sprintf(buff," TempC:%f",tempC);
 
Temperature = fnc_ds128b20_temp(0);
//Serial.println(String("Temperature")+String(Temperature)+String("C"));
  
  // If temperature goes out of regulation, send alert email
  if (tempC < 37.4 || tempC > 37.8) {
    if (tempC > 37.4) {
      gsender->Subject(subject)->Send("y1kmitlmed.arduino@gmail.com", buff);
    } else {
      gsender->Subject(subject)->Send("y1kmitlmed.arduino@gmail.com", buff);
    }
  }
  // Delay reading by 15 minute
  delay(900000);
 
}


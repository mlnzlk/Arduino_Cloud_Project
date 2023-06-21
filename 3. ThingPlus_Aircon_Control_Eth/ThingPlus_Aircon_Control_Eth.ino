#include <Arduino.h>
#include <Ethernet.h> 
#include <SPI.h>

#include <Time.h>
#include <TimeLib.h>
//#include <Timer.h>
#include <Thingplus.h>

#include <IRremote.h>
#include <IRremoteInt.h>

#define PIN_TEMP A0
#define PIN_LIGHT A1

int reportIntervalSec = 60;
/////////////////////////////////////////////////////////////////////////////
byte mac[6] = {0xAB, 0xCD, 0xef, 0x, 0x, 0x};	//FIX MAC ADDRESS

IPAddress local_ip(192,168, 0, 4);    // IP Address
IPAddress gateway(192,168, 0, 1);    // Gateway Address
IPAddress subnet(255, 255, 255, 0);    // Subnet Mask
IPAddress dns_addr(8, 8, 8, 8);    // DNS Address

const char *apikey = "API KEY";    //FIX APIKEY

const char *id_powerswitch = "powerSwitch-000000000000-0";  //FIX 
const char *id_temperature = "temperature-000000000000-0";  //FIX 
const char *id_light = "light-000000000000-0"; //FIX              
//////////////////////////////////////////////////////////////////

static EthernetClient ethernetClient;
IRsend irsend;   // Uno pin default digital 3(PWM) 
time_t current;
time_t nextReportInterval = now();
//Timer t;


void setup()
{
	initSerial();

  initEthernet();

	initThingPlus();

  nextReportInterval = now();
}


void loop()
{  
  Thingplus.loop();
//  t.update(); 

  current = now();

  if (current > nextReportInterval)
  {
    Thingplus.gatewayStatusPublish(true, reportIntervalSec * 3);

    Thingplus.sensorStatusPublish(id_powerswitch, true, reportIntervalSec * 3);

    //Thingplus.sensorStatusPublish(id_temperature, true, reportIntervalSec * 3);
    Thingplus.valuePublish(id_temperature, getTempValue(PIN_TEMP));

    //Thingplus.sensorStatusPublish(id_light, true, reportIntervalSec * 3);
    Thingplus.valuePublish(id_light, getLightValue(PIN_LIGHT));

    nextReportInterval = current + reportIntervalSec;
    Serial.println(" ===== Published to ThingPlus ===== ");
  }
}



int getTempValue(byte sensor_pin)
{
  float tmp_c = analogRead(sensor_pin);
  //LM35 : http://playground.arduino.cc/Main/LM35HigherResolution
  tmp_c = (5.0 * tmp_c * 100.0)/1024.0;

  //TMP36 : https://learn.adafruit.com/tmp36-temperature-sensor/overview
  //tmp_c = ((5.0 * tmp_c/1024.0) - 0.5) * 100;
    
  return (int)tmp_c;
}


int getLightValue(byte sensor_pin)
{
  return analogRead(sensor_pin);
}


char* actuatingCallback(const char *id, const char *cmd, JsonObject& options)
{
  if (!strcmp(id, id_powerswitch)) 
  {
    if (!strcmp(cmd, "on")) 
    {
      irsend.sendLG(0x8800909, 28);
      return "success";
    }
    else if (!strcmp(cmd, "off")) 
    {
      irsend.sendLG(0x88C0051, 28);
      return "success";
    }
  }
  return NULL;
}




static void initSerial(void)
{
  Serial.begin(9600);
  while (!Serial);// wait for serial port to connect.
  Serial.println("Start Application");
}


static void initEthernet(void) 
{
  // start the Ethernet connection:
  Serial.println(F("Trying to get an IP address using DHCP"));

  if (Ethernet.begin(mac) == 0)    // DHCP서버에 IP할당 요청  
  {
    // IP할당에 실패하면 Static으로 설정 
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    Serial.println(F("Configure Static Network Information"));
    // initialize the ethernet device not using DHCP:
    Ethernet.begin(mac, local_ip, dns_addr, gateway, subnet);
  }
    
  Serial.print(F("[INFO] Device IP address : "));
  Serial.println(Ethernet.localIP());    // IP주소 출력 

  Serial.println();
  Serial.println("*****  Arduino HomeStation *****");
  Serial.println();
}


void initThingPlus(void)
{
  Thingplus.begin(ethernetClient, mac, apikey);
  Thingplus.actuatorCallbackSet(actuatingCallback);
  Thingplus.connect();
}
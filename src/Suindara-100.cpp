/*
 _______  __   __  ___   __    _  ______   _______  ______    _______ 
|       ||  | |  ||   | |  |  | ||      | |   _   ||    _ |  |   _   |
|  _____||  | |  ||   | |   |_| ||  _    ||  |_|  ||   | ||  |  |_|  |
| |_____ |  |_|  ||   | |       || | |   ||       ||   |_||_ |       |
|_____  ||       ||   | |  _    || |_|   ||       ||    __  ||       |
 _____| ||       ||   | | | |   ||       ||   _   ||   |  | ||   _   |
|_______||_______||___| |_|  |__||______| |__| |__||___|  |_||__| |__|
  
 Name:     Suindara
 Date:     DEC 2021
 Author:   Flavio L Puhl Jr <flavio_puhl@hotmail.com> 
 GIT:      
 About:    MQTT subscribe (eclipse broker) and Thinger.io publisher 
 
Update comments                                      
+-----------------------------------------------------+------------------+---------------+
|               Feature added                         |     Version      |      Date     |
+-----------------------------------------------------+------------------+---------------+
| Initial Release                                     |      1.0.0       |     DEC/21    |
| OTA Added + Remote reset + remote console           |      2.0.0       |     JUL/22    |
|                                                     |                  |               |
+-----------------------------------------------------+------------------+---------------+


Library versions                                       
+-----------------------------------------+------------------+-------------------------- +
|       Library                           |     Version      |          Creator          |
+-----------------------------------------+------------------+-------------------------- +
| #include <ESP8266WiFi.h>                |      1.2.7       |     Arduino Community     |
| #include <PubSubClient.h>               |      2.8.0       |     Nick O'Leary          |
| #include <ArduinoJson.h>                |      6.18.5      |     Benoît Blanchon       |
| #include <NTPClient.h>                  |      3.1.0       |     Arduino Community     |
| #include <WiFiUdp.h>                    |                  |                           |
| #include <ThingerESP8266.h>             |      2.21.1      |       thinger-io          |
+-----------------------------------------+------------------+-------------------------- +


Upload settings 
+----------------------------------------------------------------------------------------+
| PLATFORM: Espressif 8266 (3.2.0) > NodeMCU 1.0 (ESP-12E Module)                        |
| HARDWARE: ESP8266 160MHz, 80KB RAM, 4MB Flash                                          |
| PACKAGES:                                                                              |
|  - framework-arduinoespressif8266 3.30002.0 (3.0.2)                                    |
|  - tool-esptool 1.413.0 (4.13)                                                         |
|  - tool-esptoolpy 1.30000.201119 (3.0.0)                                               |
|  - toolchain-xtensa 2.100300.210717 (10.3.0)                                           |
|                                                                                        |
| RAM:   [====      ]  44.1% (used 36108 bytes from 81920 bytes)                         |
| Flash: [====      ]  42.9% (used 448061 bytes from 1044464 bytes)                      |
+----------------------------------------------------------------------------------------+

*/

// Uncomment the line below if you wish to use OTA updates
#define USE_OTA

/*+--------------------------------------------------------------------------------------+
 *| Libraries                                                                            |
 *+--------------------------------------------------------------------------------------+ */
// Libraries built into IDE
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define _DISABLE_TLS_                                 // Needed for Thinger
#include <ThingerESP8266.h>                           // Thinger
#define THINGER_SERIAL_DEBUG
#include <ThingerConsole.h>

#ifdef USE_OTA

  #include <ESP8266WiFiMulti.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266httpUpdate.h>
  
  #define OTAFIRMWAREREPO "https://firebasestorage.googleapis.com/v0/b/firmwareota-a580e.appspot.com/o/Suindara%2Ffirmware.bin?alt=media"
  bool taskCompleted = false;

#endif

/*+--------------------------------------------------------------------------------------+
 *| Constants declaration                                                                |
 *+--------------------------------------------------------------------------------------+ */
 
const char *ssid =  "CasaDoTheodoro1";                       // name of your WiFi network
const char *password =  "09012011";                          // password of the WiFi network

const char *ID =      "SuindaraDev";                         // Name of our device, must be unique
const char *TOPIC1 =  "Seriema/data";                        // Topic to subcribe to
const char *TOPIC2 =  "Harpia/data";                         // Topic to subcribe to
const char *TOPIC3 =  "Narceja/data";                        // Topic to subcribe to
const char *TOPIC4 =  "Duinofarm/data";                      // Topic to subcribe to

const char* BROKER_MQTT = "broker.hivemq.com";
//const char* BROKER_MQTT = "mqtt.eclipseprojects.io";          // MQTT Cloud Broker URL


String swversion = __FILE__;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WiFiClient wclient;
PubSubClient client(wclient);                                 /* Setup MQTT client */

#define user "fpuhl"                                          /* Thinger */
#define device_Id "HomenoideGateway"                          /* Thinger */
#define device_credentials "jt0J73!BihBvbXe7"                 /* Thinger */
ThingerESP8266 thing(user, device_Id, device_credentials);    /* Thinger */
ThingerConsole console(thing);

/*+--------------------------------------------------------------------------------------+
 *| Global Variables                                                                     |
 *+--------------------------------------------------------------------------------------+ */

unsigned long loop1 = 0;                             /* stores the value of millis() in each iteration of loop() */
unsigned long loop2 = 0; 

float uptime = 0;
int REMreset = 0;

char const* Device_topic0;
String      Version_topic0        = "empty";
float       RSSI_topic0           = 0;
String      IP_topic0             = "empty";
String      LastRoll_topic0       = "empty";
int         Uptime_topic0         = 0;

String      Device_topic1str      = "empty"; 
String      Version_topic1str     = "empty";
float       RSSI_topic1           = 0;
String      IP_topic1str          = "empty";
String      LastRoll_topic1str    = "empty";
int         Uptime_topic1         = -1;
float       Temp_topic1           = 0;

String      Device_topic2str      = "empty";
String      Version_topic2str     = "empty";
float       RSSI_topic2           = 0;
String      IP_topic2str          = "empty";
String      LastRoll_topic2str    = "empty";
int         Uptime_topic2         = -1;
char const* LastPicture_topic2    = "empty";
float       Temp_topic2           = 0;

String      Device_topic3str      = "empty";
String      Version_topic3str     = "empty";
float       RSSI_topic3           = 0;
String      IP_topic3str          = "empty";
String      LastRoll_topic3str    = "empty";
int         Uptime_topic3         = -1;
char const* WakeUpReason_topic3   = "empty";
float       MeasCounter_topic3    = 0;
float       BattVoltLvl_topic3    = 0;

String      Device_topic4str      = "empty";
String      Version_topic4str     = "empty";
float       RSSI_topic4           = 0;
String      IP_topic4str          = "empty";
String      LastRoll_topic4str    = "empty";
int         Uptime_topic4         = -1;
float       Totalbalance_topic4   = 0;
float       Totalminers_topic4    = 0;
float       Totalhashrate_topic4  = 0;


/*+--------------------------------------------------------------------------------------+
 *| Serial debug                                                                         |
 *+--------------------------------------------------------------------------------------+ */

void debug_Topic0(){
  
  Serial.print(" Topic0 Device    = ");    Serial.println(Device_topic0);
  Serial.print(" Topic0 Version   = ");    Serial.println(Version_topic0);
  Serial.print(" Topic0 RSSI      = ");    Serial.println(RSSI_topic0);
  Serial.print(" Topic0 IP        = ");    Serial.println(IP_topic0);
  Serial.print(" Topic0 LastRoll  = ");    Serial.println(LastRoll_topic0);
  Serial.print(" Topic0 Uptime    = ");    Serial.println(Uptime_topic0);
  Serial.println();

}

void debug_Topic1(){

  Serial.print(" Topic1 Device    = ");    Serial.println(Device_topic1str);
  Serial.print(" Topic1 Version   = ");    Serial.println(Version_topic1str);
  Serial.print(" Topic1 RSSI      = ");    Serial.println(RSSI_topic1);
  Serial.print(" Topic1 IP        = ");    Serial.println(IP_topic1str);
  Serial.print(" Topic1 LastRoll  = ");    Serial.println(LastRoll_topic1str);
  Serial.print(" Topic1 Uptime    = ");    Serial.println(Uptime_topic1);
  Serial.print(" Topic1 Temp      = ");    Serial.println(Temp_topic1);
  Serial.println();
}


void debug_Topic2(){

  Serial.print(" Topic2 Device    = ");    Serial.println(Device_topic2str);
  Serial.print(" Topic2 Version   = ");    Serial.println(Version_topic2str);
  Serial.print(" Topic2 RSSI      = ");    Serial.println(RSSI_topic2);
  Serial.print(" Topic2 IP        = ");    Serial.println(IP_topic2str);
  Serial.print(" Topic2 LastRoll  = ");    Serial.println(LastRoll_topic2str);
  Serial.print(" Topic2 Uptime    = ");    Serial.println(Uptime_topic2);
  Serial.print(" Topic2 Temp      = ");    Serial.println(Temp_topic2);
  Serial.print(" Topic2 LastPic   = ");    Serial.println(LastPicture_topic2);
  Serial.println();

}

void debug_Topic3(){

  Serial.print(" Topic3 Device    = ");    Serial.println(Device_topic3str);
  Serial.print(" Topic3 Version   = ");    Serial.println(Version_topic3str);
  Serial.print(" Topic3 RSSI      = ");    Serial.println(RSSI_topic3);
  Serial.print(" Topic3 IP        = ");    Serial.println(IP_topic3str);
  Serial.print(" Topic3 LastRoll  = ");    Serial.println(LastRoll_topic3str);
  Serial.print(" Topic3 Uptime    = ");    Serial.println(Uptime_topic3);
  Serial.print(" Topic3 WakeUp    = ");    Serial.println(WakeUpReason_topic3);
  Serial.print(" Topic3 MeasCntr  = ");    Serial.println(MeasCounter_topic3);
  Serial.print(" Topic3 BattLvl   = ");    Serial.println(BattVoltLvl_topic3);
  Serial.println();

}

void debug_Topic4(){
  
  Serial.print(" Topic4 Device    = ");    Serial.println(Device_topic4str);
  Serial.print(" Topic4 Version   = ");    Serial.println(Version_topic4str);
  Serial.print(" Topic4 RSSI      = ");    Serial.println(RSSI_topic4);
  Serial.print(" Topic4 IP        = ");    Serial.println(IP_topic4str);
  Serial.print(" Topic4 LastRoll  = ");    Serial.println(LastRoll_topic4str);
  Serial.print(" Topic4 Uptime    = ");    Serial.println(Uptime_topic4);
  Serial.print(" Topic4 Totalbal  = ");    Serial.println(Totalbalance_topic4);
  Serial.print(" Topic4 TtMiners  = ");    Serial.println(Totalminers_topic4);
  Serial.print(" Topic4 Tt hash   = ");    Serial.println(Totalhashrate_topic4);
  Serial.println();

}


/*+--------------------------------------------------------------------------------------+
 *| MQTT callback                                                                        |
 *+--------------------------------------------------------------------------------------+ */

// Handle incomming messages from the broker
void callback(char* topic, byte* payload, unsigned int length) {

  char str[length+1] = ".";
    Serial.printf("Message arrived [ %s ]\n",topic);

  unsigned int i=0;
  for (i=0;i<length;i++) {
    str[i]=(char)payload[i];
    //Serial.print((char)payload[i]);                               // print raw json data, debug only
    //Serial.print(str[i]);                                           // print raw json data, debug only
  }
  
  Serial.println();       
  str[i] = 0; // Null termination

 // JSON Deserialization
  StaticJsonDocument <1000> doc;
  
  DeserializationError error = deserializeJson(doc, payload);
  
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str()); //https://github.com/bblanchon/ArduinoJson/issues/1525
      return;
    } else {
      Serial.print(F("deserializeJson() success.. "));
    }

  if (strcmp(topic,TOPIC1)==0) {

    Serial.printf("Allocating data for [ %s ]\n",topic);

    Device_topic1str        =     String(doc["Device"]);
    Version_topic1str       =     String(doc["Version"]);
    RSSI_topic1             =            doc["RSSI (db)"];
    IP_topic1str            =     String(doc["IP"]);
    LastRoll_topic1str      =     String(doc["LastRoll"]);
    Uptime_topic1           =            doc["UpTime (h)"];
    Temp_topic1             =            doc["Temp (°C)"];

    debug_Topic1();
    
    if(console)                                      
      {  
      console.printf("Topic1 Device    = ");    console.printf(doc["Device"]);     console.printf("\n");
      console.printf("Topic1 Version   = ");    console.printf(doc["Version"]);    console.printf("\n");
      console.printf("Topic1 LastRoll  = ");    console.printf(doc["LastRoll"]);   console.printf("\n");
      }
  
  }

  if (strcmp(topic,TOPIC2)==0) {

    Serial.printf("Allocating data for [ %s ]\n",topic);

    Device_topic2str        =     String(doc["Device"]);
    Version_topic2str       =     String(doc["Version"]);
    RSSI_topic2             =            doc["RSSI (db)"];
    IP_topic2str            =     String(doc["IP"]);
    LastRoll_topic2str      =     String(doc["LastRoll"]);
    Uptime_topic2           =            doc["UpTime (h)"];
    LastPicture_topic2      =            doc["Last Picture"];
    Temp_topic2             =            doc["Temp (°C)"];

    debug_Topic2();

    if(console)                                      
      {  
      console.printf("Topic2 Device    = ");    console.printf(doc["Device"]);    console.printf("\n");
      console.printf("Topic2 Version   = ");    console.printf(doc["Version"]);   console.printf("\n");
      console.printf("Topic2 LastRoll  = ");    console.printf(doc["LastRoll"]);   console.printf("\n");
      }

    } 


   if (strcmp(topic,TOPIC3)==0) {

    Serial.printf("Allocating data for [ %s ]\n",topic);

    Device_topic3str        =     String(doc["Device"]);
    Version_topic3str       =     String(doc["Version"]);
    RSSI_topic3             =            doc["RSSI (db)"];
    IP_topic3str            =     String(doc["IP"]);
    LastRoll_topic3str      =     String(doc["LastRoll"]);
    Uptime_topic3           =            doc["UpTime (h)"];
    WakeUpReason_topic3     =            doc["WakeUpReason"];
    MeasCounter_topic3      =            doc["MeasCounter"];
    BattVoltLvl_topic3      =            doc["BattVoltLvl"];

    debug_Topic3();

    }   

     if (strcmp(topic,TOPIC4)==0) {

    Serial.printf("Allocating data for [ %s ]\n",topic);

    Device_topic4str        =     String(doc["Device"]);
    Version_topic4str       =     String(doc["Version"]);
    RSSI_topic4             =            doc["RSSI (db)"];
    IP_topic4str            =     String(doc["IP"]);
    LastRoll_topic4str      =     String(doc["LastRoll"]);
    Uptime_topic4           =            doc["UpTime (h)"];
    Totalbalance_topic4     =            doc["Total balance"];
    Totalminers_topic4      =            doc["Total miners"];
    Totalhashrate_topic4    =            doc["Total hashrate"];

    debug_Topic4();

    }   

}

/*+--------------------------------------------------------------------------------------+
 *| OTA Callbacks                                                                        |
 *+--------------------------------------------------------------------------------------+ */

#ifdef USE_OTA

  void update_started() {
    Serial.println("HTTP update process started");
    console.printf("HTTP update process started\r\n");
  }

  void update_finished() {
    Serial.println("HTTP update process finished, restarting");
  }

  void update_progress(int cur, int total) {
    Serial.printf("HTTP update process at %d of %d bytes...\n", cur, total);   
  }

  void update_error(int err) {
    Serial.printf("HTTP update fatal error code %d\n", err);
    console.printf("HTTP update fatal error code %d\n\r", err);
    delay(1000);
  }
 
  void update_OTA() {
  // Check for WiFi connection
  if ((WiFi.status() == WL_CONNECTED) && !taskCompleted) {

    taskCompleted = true;

    WiFiClientSecure OTAclient;                                    // Works for Plataformio
    OTAclient.setInsecure();                                       //https://maakbaas.com/esp8266-iot-framework/logs/https-requests/

    // Add optional callback notifiers
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret = ESPhttpUpdate.update(OTAclient, OTAFIRMWAREREPO);
     
    switch (ret) {
      case HTTP_UPDATE_FAILED: 
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES: 
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK: 
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  }
  }
 

#endif

/*+--------------------------------------------------------------------------------------+
 *| Connect to WiFi network                                                              |
 *+--------------------------------------------------------------------------------------+ */

void setup_wifi() {
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
    WiFi.mode(WIFI_STA);                              // Setup ESP in client mode
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.begin(ssid, password);                       // Connect to network

    int wait_passes = 0;
    while (WiFi.status() != WL_CONNECTED) {           // Wait for connection
      delay(500);
      Serial.print(".");
      if (++wait_passes >= 20) { ESP.restart(); }     // Restart in case of no wifi connection   
    }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  
}

/*+--------------------------------------------------------------------------------------+
 *| Reconnect to MQTT client                                                             |
 *+--------------------------------------------------------------------------------------+ */
 
void reconnect() {
  
  while (!client.connected()) {                             // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      

      if(client.subscribe(TOPIC1)){
        Serial.printf("Subscribing to: ");       
      } else  {
        Serial.print("FAIL to subscribe to: ");
      } 
      Serial.println(TOPIC1);

      if(client.subscribe(TOPIC2)){
        Serial.printf("Subscribing to: ");       
      } else  {
        Serial.print("FAIL to subscribe to: ");
      } 
      Serial.println(TOPIC2);

      if(client.subscribe(TOPIC3)){
        Serial.printf("Subscribing to: ");       
      } else  {
        Serial.print("FAIL to subscribe to: ");
      } 
      Serial.println(TOPIC3);

      if(client.subscribe(TOPIC4)){
        Serial.printf("Subscribing to: ");       
      } else  {
        Serial.print("FAIL to subscribe to: ");
      } 
      Serial.println(TOPIC4);

    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      setup_wifi();
    }
  }
}

/*+--------------------------------------------------------------------------------------+
 *| Get Date & Time                                                                      |
 *+--------------------------------------------------------------------------------------+ */
 
String DateAndTime(){

    timeClient.setTimeOffset(-10800);                       // Set offset time in seconds to adjust for your timezone, for example:
                                                            // GMT +1 = 3600
                                                            // GMT +8 = 28800
                                                            // GMT -1 = -3600
                                                            // GMT 0 = 0
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }

  time_t epochTime = timeClient.getEpochTime();              // The time_t type is just an integer. 
                                                             // It is the number of seconds since the Epoch.
  struct tm * tm = localtime(&epochTime);
  char dts[22];
    strftime(dts, sizeof(dts), "%d%b%Y %H-%M-%S", tm);       // https://www.cplusplus.com/reference/ctime/strftime/
  
  return dts;
 
}

/*+--------------------------------------------------------------------------------------+
 *| Setup                                                                                |
 *+--------------------------------------------------------------------------------------+ */
 
void setup() {
  Serial.begin(115200);                               // Start serial communication at 115200 baud
    delay(500);
  
  pinMode(2, OUTPUT);                                 // Buit-in led [GPIO02 or D4]
  
  swversion = (swversion.substring((swversion.indexOf(".")), (swversion.lastIndexOf("\\")) + 1))+" "+__DATE__+" "+__TIME__;   
   Serial.printf("\nSW version: ");
   Serial.println(swversion);
     
  setup_wifi();                                       // Connect to network

  Serial.println("Broker MQTT setting server");	
  client.setServer(BROKER_MQTT, 1883);
         
  Serial.println("Starting timeclient server"); 	    // Initialize a NTPClient to get time
  timeClient.begin();             

  Serial.println("Initialize MQTT callback routine"); 	
  client.setCallback(callback);                       // Initialize the callback routine

  
  // digital pin control
  thing["BuiltInLed"] << digitalPin(2);               // built-in led

  thing["reset"] << inputValue(REMreset);             // int value returned from Thinger.io

  Serial.println("Declaring Thinger variables.. ");
    thing["data"] >> [](pson& out){  
     // Add the values and the corresponding code
      out["Device_topic0"]          = Device_topic0;
      out["Version_topic0"]         = Version_topic0;
      out["RSSI_topic0"]            = RSSI_topic0;
      out["IP_topic0"]              = IP_topic0;
      out["LastRoll_topic0"]        = LastRoll_topic0;
      out["Uptime_topic0"]          = Uptime_topic0;

      out["Device_topic1"]          = Device_topic1str;
      out["Version_topic1"]         = Version_topic1str;
      out["RSSI_topic1"]            = RSSI_topic1;
      out["IP_topic1"]              = IP_topic1str;
      out["LastRoll_topic1"]        = LastRoll_topic1str;
      out["Uptime_topic1"]          = Uptime_topic1;
      out["Temp"]                   = Temp_topic1;

      out["Device_topic2"]          = Device_topic2str;
      out["Version_topic2"]         = Version_topic2str;
      out["RSSI_topic2"]            = RSSI_topic2;
      out["IP_topic2"]              = IP_topic2str;
      out["LastRoll_topic2"]        = LastRoll_topic2str;
      out["Uptime_topic2"]          = Uptime_topic2;
      out["Temp_topic2"]            = Temp_topic2;
      out["LastPic_topic2"]         = LastPicture_topic2;

      out["Device_topic3"]          = Device_topic3str;
      out["Version_topic3"]         = Version_topic3str;
      out["RSSI_topic3"]            = RSSI_topic3;
      out["IP_topic3"]              = IP_topic3str;
      out["LastRoll_topic3"]        = LastRoll_topic3str;
      out["Uptime_topic3"]          = Uptime_topic3;
      out["WakeReason_topic3"]      = WakeUpReason_topic3;
      out["MeasCntr_topic3"]        = MeasCounter_topic3;
      out["BattLvl_topic3"]         = BattVoltLvl_topic3;

      out["Device_topic4"]          = Device_topic4str;
      out["Version_topic4"]         = Version_topic4str;
      out["RSSI_topic4"]            = RSSI_topic4;
      out["IP_topic4"]              = IP_topic4str;
      out["LastRoll_topic4"]        = LastRoll_topic4str;
      out["Uptime_topic4"]          = Uptime_topic4;
      out["Totalbalance_topic4"]    = Totalbalance_topic4;
      out["Totalminers_topic4"]     = Totalminers_topic4;
      out["Totalhashrate_topic4"]   = Totalhashrate_topic4;

    }; 

}

/*+--------------------------------------------------------------------------------------+
 *| main loop                                                                            |
 *+--------------------------------------------------------------------------------------+ */
 
void loop() {

  unsigned long currentMillis = millis();            // capture the latest value of millis() 
  uptime = millis()/3600000;                         // Update uptime 
  
  
  if (currentMillis - loop1 >= 1000) {               // Gateway heartbeat every 1s

    if(console)                                      // Check if Thinger remote console is connected
      {  
        console.printf("Console connected:       %lu\r\n", currentMillis);
      } 

    if (!client.connected())                         // Reconnect if connection to MQTT is lost
      {  
        console.printf("MQTT not connected:      %lu\r\n", currentMillis);
        reconnect();
      } else {        
        console.printf("MQTT connected:          %lu\r\n", currentMillis);
      }

    if (REMreset >= 1) 
      { 
        console.printf("Reset Command received:  %lu\r\n", currentMillis);
        delay(1000);
        console.printf("Search for firmware update before restart");
        update_OTA();
        ESP.restart();                              // Restart remote from thinger.io
      }  
          
  loop1 = currentMillis;
  } 

  
  if (currentMillis - loop2 >= 1*60*1000) {         // Gateway device health check
   Serial.println("Loop Main: Start");
   
    Device_topic0     = "Suindara";
    Version_topic0    = swversion;
    RSSI_topic0       = WiFi.RSSI();
    IP_topic0         = WiFi.localIP().toString();
    LastRoll_topic0   = DateAndTime();
    Uptime_topic0     = uptime;

    debug_Topic0();
    debug_Topic1();
    debug_Topic2();
    debug_Topic3();
    debug_Topic4();
    
    loop2 = currentMillis;
  }      

  thing.handle();                                 // Thinger  

  client.loop();                                  // MQTT
 
}


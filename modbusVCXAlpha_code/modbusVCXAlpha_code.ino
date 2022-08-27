/**
 * WiFiManagerParameter child class example
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <ESP_EEPROM.h>
//#include "Pushover.h"
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 1
#define FIRST_REG 18
#define REG_COUNT 10
#define RXTX_PIN 2

SoftwareSerial S(D2, D1);
ModbusRTU mb;
WiFiClient espClient;

unsigned long myChannelNumber = 0;
const char * myWriteAPIKey = "0";

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Callback to monitor errors
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Request result: 0x");
    Serial.print(event, HEX);
  }
  return true;
}

#define SETUP_PIN 14
#define HIGH_LVL_PIN 5
#define LOW_LVL_PIN 4
bool petla = false;
bool notification_1 = 0;
bool notification_1_1 = 0;
bool notification_2 = 0;
bool notification_2_1 = 0;
unsigned long previousMillis = 0; 
unsigned long interval = 1000;  
class IPAddressParameter : public WiFiManagerParameter {
public:
    IPAddressParameter(const char *id, const char *placeholder, IPAddress address)
        : WiFiManagerParameter("") {
        init(id, placeholder, address.toString().c_str(), 16, "", WFM_LABEL_BEFORE);
    }

    bool getValue(IPAddress &ip) {
        return ip.fromString(WiFiManagerParameter::getValue());
    }
};

class IntParameter : public WiFiManagerParameter {
public:
    IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    long getValue() {
        return String(WiFiManagerParameter::getValue()).toInt();
    }
};

class FloatParameter : public WiFiManagerParameter {
public:
    FloatParameter(const char *id, const char *placeholder, float value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    float getValue() {
        return String(WiFiManagerParameter::getValue()).toFloat();
    }
};

struct Settings {
    float f;
    int i;
    unsigned long chanID;
    char s[31];
    char usr_key[31];
    char com1[31];
    char com11[31];
    char com2[31];
    char com21[31];
    uint32_t ip;
} sett;


void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    pinMode(SETUP_PIN, INPUT_PULLUP);
    pinMode(HIGH_LVL_PIN, INPUT_PULLUP);
    pinMode(LOW_LVL_PIN, INPUT_PULLUP);
    Serial.begin(115200); 
      S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S, RXTX_PIN);
  mb.master();
  Serial.println("Inicjalizacja modbus");

    //Delay to push SETUP button
    Serial.println("Press setup button");
    for (int sec = 3; sec > 0; sec--) {
        Serial.print(sec);
        Serial.print("..");
        delay(1000);
    }

    EEPROM.begin( 1024 );
    delay(100);
    EEPROM.get(0, sett);
    
    Serial.println("Settings loaded");
    
    if (digitalRead(SETUP_PIN) == LOW) {  
        // Button pressed 
        Serial.println("SETUP");

        WiFiManager wm;
        
        sett.s[30] = '\0';   //add null terminator at the end cause overflow
        
        sett.usr_key[30] = '\0';
        sett.com1[30] = '\0';
        sett.com11[30] = '\0';
        sett.com2[30] = '\0';
        sett.com21[30] = '\0';
        WiFiManagerParameter param_str( "str", "param_string",  sett.s, 31);
        IntParameter param_chanID( "chanID", "param_chanID",  sett.chanID);
        WiFiManagerParameter param_user_key( "str1", "param_user_key",  sett.usr_key, 31);
        WiFiManagerParameter param_text1( "str2", "param_com1",  sett.com1, 31);
        WiFiManagerParameter param_text11( "str21", "param_com11",  sett.com11, 31);
        WiFiManagerParameter param_text2( "str3", "param_com2",  sett.com2, 31);
        WiFiManagerParameter param_text21( "str31", "param_com21",  sett.com21, 31);
        FloatParameter param_float( "float", "param_float",  sett.f);
        IntParameter param_int( "int", "param_int",  sett.i);

        IPAddress ip(sett.ip);
        IPAddressParameter param_ip("ip", "param_ip", ip);
WiFiManagerParameter custom_text1("<p>Proszę uzupełnić parametry powiadomien Pushover</p>");
wm.addParameter(&custom_text1);
        wm.addParameter( &param_str );
        WiFiManagerParameter custom_text3("<p>Proszę uzupełnić channel ID</p>");
        wm.addParameter(&custom_text3);
        wm.addParameter( &param_chanID );
        wm.addParameter( &param_user_key );
        wm.addParameter( &param_text1 );
        wm.addParameter( &param_text11 );
        wm.addParameter( &param_text2 );
        wm.addParameter( &param_text21 );
        wm.addParameter( &param_float );
WiFiManagerParameter custom_text2("<p>Proszę podać czas próbkowania(czas pomiędzy sprawdzeniami)(s)</p>");
wm.addParameter(&custom_text2);        
        wm.addParameter( &param_int );
        wm.addParameter( &param_ip );
        WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
wm.addParameter(&custom_text);
Serial.println("Linijka przed commit");
        //SSID & password parameters already included
        wm.startConfigPortal();
Serial.println("Linijka przed commit");
        strncpy(sett.s, param_str.getValue(), 31);
        sett.s[30] = '\0'; 
        strncpy(sett.usr_key, param_user_key.getValue(), 31);
        sett.usr_key[30] = '\0'; 
        strncpy(sett.com1, param_text1.getValue(), 31);
        sett.com1[30] = '\0'; 
        strncpy(sett.com11, param_text11.getValue(), 31);
        sett.com11[30] = '\0';         
        strncpy(sett.com2, param_text2.getValue(), 31);
        sett.com2[30] = '\0';
        strncpy(sett.com21, param_text21.getValue(), 31);
        sett.com21[30] = '\0';        
        sett.f = param_float.getValue();
        sett.i = param_int.getValue();
        sett.chanID = param_chanID.getValue();

        Serial.print("123String param: ");
        Serial.println(sett.s);
         Serial.print("User key: ");
        Serial.println(sett.usr_key);
        Serial.print("Float param: ");
        Serial.println(sett.f);
        Serial.print("Int param: ");
        Serial.println(sett.i, DEC);
        Serial.print("ChannelID: ");
        Serial.println(sett.chanID, DEC);
        
        if (param_ip.getValue(ip)) {
            sett.ip = ip;

            Serial.print("IP param: ");
            Serial.println(ip);
        } else {
            Serial.println("Incorrect IP");
        }
Serial.println("Linijka przed commit");
        EEPROM.put(0, sett);
        if (EEPROM.commit()) {
          Serial.println("Zapisano");
            Serial.println("Settings saved");
        } else {
            Serial.println("EEPROM error");
        }
    } 
    else {  
        Serial.println("WORK");

        //connect to saved SSID
        WiFi.begin();  

        //do smth
        Serial.print("String param: ");
        Serial.println(sett.s);
         Serial.print("User key: ");
        Serial.println(sett.usr_key);
        Serial.print("text1: ");
        Serial.println(sett.com1);
        Serial.print("text2: ");
        Serial.println(sett.com2);
        Serial.print("Float param: ");
        Serial.println(sett.f);
        Serial.print("Int param: ");
        Serial.println(sett.i, DEC);
        Serial.print("ChanID: ");
        Serial.println(sett.chanID, DEC);
        Serial.print("IP param: ");
        IPAddress ip(sett.ip);
        Serial.println(ip);
    }
 while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(espClient);
    
}

void loop() {

  if(petla==0){
  //Serial.begin(115200);
  //WiFi.begin("myNetwork", "secureWPAKii");
  while (WiFi.status() != WL_CONNECTED) delay(50);
  Serial.println("Connected");
/*  
  Pushover po = Pushover(sett.s,sett.usr_key, UNSAFE);
  po.setDevice("test");
  po.setMessage("TEST-.");
  //Powiadomienie testowe. Urządzenie uruchomione poprawnie
  po.setSound("default");
  Serial.println(po.send()); //should return 1 on success
  */
  petla=1;}

  interval=sett.i*1000;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
  uint16_t res[REG_COUNT];
  if (!mb.slave()) {    // Check if no transaction in progress
    mb.readHreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }

    union
   {
    uint32_t j;
    float f;
   }u;
    Serial.println("Rejestry2");
    Serial.println(res[0]);
    Serial.println(res[1]);
    Serial.println("float");
    u.j =((unsigned long)res[0] << 16) | res[1]; //A phase active power
    Serial.println(u.f);
    ThingSpeak.setField(1,u.f );
    u.j =(((unsigned long)res[2] << 16) | res[3])+(((unsigned long)res[4] << 16) | res[5]); //B+C phase active power
    Serial.println(u.f);
    ThingSpeak.setField(2,u.f );
    u.j =((unsigned long)res[6] << 16) | res[7]; //total active power
    Serial.println(u.f);
    ThingSpeak.setField(3,u.f );
      if (!mb.slave()) {    // Check if no transaction in progress
    mb.readHreg(SLAVE_ID, FIRST_REG+34, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }

        Serial.println("Rejestry");
    Serial.println(res[0]);
    Serial.println(res[1]);
    u.j =((unsigned long)res[0] << 16) | res[1]; //Positive active power kWh
    Serial.println(u.f);
    ThingSpeak.setField(4,u.f );









    
  int x = ThingSpeak.writeFields(sett.chanID, sett.s); //Write Api kay is in S variable
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  //ThingSpeak.setField(3, number3);
 // ThingSpeak.setField(4, number4);
  }
  }
  }





/*   
    if(digitalRead(HIGH_LVL_PIN)==0&&notification_1==0){
      notification_1=1;
     // Pushover po = Pushover(sett.s,sett.usr_key, UNSAFE); 
       po.setDevice("test");
       po.setMessage(sett.com1);
       po.setSound("default");
       Serial.println(po.send());
     }
    if(notification_1==1&&digitalRead(HIGH_LVL_PIN)==1){notification_1=0;}
    
    if(digitalRead(HIGH_LVL_PIN)==1&&notification_1_1==0){
      notification_1_1=1;
     // Pushover po = Pushover(sett.s,sett.usr_key, UNSAFE); 
       po.setDevice("test");
       po.setMessage(sett.com11);
       po.setSound("default");
       Serial.println(po.send());
     }
    if(notification_1==1&&digitalRead(HIGH_LVL_PIN)==0){notification_1_1=0;}
///////////////////////////////////////////////////////////////////////////////////////////
        if(digitalRead(LOW_LVL_PIN)==0&&notification_2==0){
      notification_2=1;
     // Pushover po = Pushover(sett.s,sett.usr_key, UNSAFE); 
       po.setDevice("test");
       po.setMessage(sett.com2);
       po.setSound("default");
       Serial.println(po.send());
     }
    if(notification_2==1&&digitalRead(LOW_LVL_PIN)==1){notification_2=0;}
    //////////////////////////////////////////////////
    if(digitalRead(LOW_LVL_PIN)==1&&notification_2_1==0){
      notification_2_1=1;
     // Pushover po = Pushover(sett.s,sett.usr_key, UNSAFE); 
    //  po.setDevice("test");
      // po.setMessage(sett.com21);
      // po.setSound("default");
      // Serial.println(po.send());
     }
    if(notification_2_1==1&&digitalRead(LOW_LVL_PIN)==0){notification_2_1=0;}    
    
  }
*/
  ArduinoOTA.handle();  
}

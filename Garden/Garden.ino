#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define retained true

const long utcOffsetInSeconds = 16200;                                                                            //+3:30 UTC Tehran
String clientId = String(ESP.getChipId());
int failcount;
const char *mqtt_server = "192.168.1.3";
const char *local_server = "arad-pc.local";                                                                       //MQTT server address(add A/CNAME in hosts
int today = -1 , hp1 = -1 ,hp2 = -1 ,hls = -1 ,hle = -1, mp1 = -1 , mp2 = -1, mls = -1 , mle = -1, volume = 0;

WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void callback(char* topic, byte* payload, unsigned int length)
{
  String T = topic;
  if(T == "Garden/Pump")
  {
    digitalWrite(1 , !(payload[0] - '0'));
    client.publish("Garden/Pump/Status", String(!digitalRead(1)).c_str() , retained);
  }
  if(T == "Garden/Light")
  { 
    digitalWrite(3 , !(payload[0] - '0'));
    client.publish("Garden/Light/Status", String(!digitalRead(3)).c_str() , retained);
  }
  else if(T == "Garden/Pump/Volume")
  {
    int temp = 0;
    for(int i = 0 ; i < length ; i++)
    {
      temp *= 10;
      temp += payload[i] - '0';
    }
    volume = temp;
  }
  else if(T == "Garden/Pump/Time1")
  {
    int h = 0 , m = 0;
    bool toggle = false;
    for(int i = 0 ; i < length ; i++)
      {
        if(toggle)
        {
          m *= 10;
          m += payload[i] - '0';
        }
        if(payload[i] == ':')
          toggle = true;
        if(!toggle)
        {
          h *= 10;
          h += payload[i] - '0';
        }
      }
    hp1 = h;
    mp1 = m;
  }
  else if(T == "Garden/Pump/Time2")
  {
    int h = 0 , m = 0;
    bool toggle = false;
    for(int i = 0 ; i < length ; i++)
      {
        if(toggle)
        {
          m *= 10;
          m += payload[i] - '0';
        }
        if(payload[i] == ':')
          toggle = true;
        if(!toggle)
        {
          h *= 10;
          h += payload[i] - '0';
        }
      }
    hp2 = h;
    mp2 = m;
  }
  else if(T == "Garden/Light/Start")
  {
    int h = 0 , m = 0;
    bool toggle = false;
    for(int i = 0 ; i < length ; i++)
      {
        if(toggle)
        {
          m *= 10;
          m += payload[i] - '0';
        }
        if(payload[i] == ':')
          toggle = true;
        if(!toggle)
        {
          h *= 10;
          h += payload[i] - '0';
        }
      }
    hls = h;
    mls = m;
  }
  else if(T == "Garden/Light/End")
  {
    int h = 0 , m = 0;
    bool toggle = false;
    for(int i = 0 ; i < length ; i++)
      {
        if(toggle)
        {
          m *= 10;
          m += payload[i] - '0';
        }
        if(payload[i] == ':')
          toggle = true;
        if(!toggle)
        {
          h *= 10;
          h += payload[i] - '0';
        }
      }
    hle = h;
    mle = m;
  }
}

void reconnect() 
{
  String msg;
  while (!client.connected())
  {
    msg = "ESP-" + clientId + " joined.";
    if (client.connect(clientId.c_str() , "Garden/online" , 2 , retained , "0"))
    {
      client.subscribe("Garden/Pump");
      client.subscribe("Garden/Light");
      client.publish("Garden/online", "1" , retained);
      client.publish("ack", msg.c_str());
    }
    else 
    { 
      failcount++;
      if(failcount % 2)
        client.setServer(local_server, 1883);
      else 
        client.setServer(mqtt_server,1883);
    }
  }
}

void setup()
{
  pinMode(1 , OUTPUT);
  digitalWrite(1 , HIGH);
  pinMode(3 , OUTPUT);
  digitalWrite(3 , HIGH);
  WiFi.hostname("Garden");
  wifiManager.setTimeout(180);
  wifiManager.autoConnect("Garden");
  ArduinoOTA.setHostname("Garden");
  ArduinoOTA.begin();
  timeClient.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  client.publish("Garden/Pump/Status", String(!digitalRead(0)).c_str() , retained);
  client.publish("Garden/Light/Status", String(!digitalRead(0)).c_str() , retained);
}

long long int mil = millis();
bool done1 = 0 , done2 = 0;

void loop()
{
  if(millis() - mil > 60000)
  {
    mil = millis();
    timeClient.update();
    if(client.connected())
    {
      client.subscribe("Garden/Pump/Time1");
      client.subscribe("Garden/Pump/Time2");
      client.subscribe("Garden/Pump/Volume");
      client.subscribe("Garden/Light/Start");
      client.subscribe("Garden/Light/End");
    }
    if((timeClient.getHours() == hp1 && timeClient.getMinutes() == mp1) || (timeClient.getHours() == hp2 && timeClient.getMinutes() == mp2))
    {
      digitalWrite(3 ,HIGH);                                                          //turn off Light power surge high
      digitalWrite(1 ,LOW);
      client.publish("Garden/Pump/Status", String(!digitalRead(1)).c_str() , retained);
      delay(volume * 1000);
      digitalWrite(1 ,HIGH);
      client.publish("Garden/Pump/Status", String(!digitalRead(1)).c_str() , retained);
    }
    if((timeClient.getHours() > hls && timeClient.getHours() < (hls > hle ? 24 : hle)) || (timeClient.getHours() > (hls > hle ? 0 : hls) && timeClient.getHours() < hle) || (timeClient.getHours() == hls && timeClient.getMinutes() >= mls) || (timeClient.getHours() == hle && timeClient.getMinutes() <= mle))
    {
      digitalWrite(3 ,LOW);
      client.publish("Garden/Light/Status", String(!digitalRead(3)).c_str() , retained);
    }
    else
    {
      digitalWrite(3 ,HIGH);
      client.publish("Garden/Light/Status", String(!digitalRead(3)).c_str() , retained);
    }
  }
  ArduinoOTA.handle();
  if(!client.connected())
    reconnect();
  client.loop();
}

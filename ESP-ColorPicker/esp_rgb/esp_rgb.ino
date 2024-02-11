#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <ArduinoOTA.h>
#include <WebSerial.h>
#include "cie1931.h"


#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

void update_started();
void update_progress(int progress, int total);
void update_finished();
void rgb(int r, int g, int b, const uint32_t table[]);
void handle_fade_rgb();
void handle_set_rgb();

// Set web server port number to 80
ESP8266WebServer server(80);
int last_progress = 0;

// Red, green, and blue pins for PWM control
const int redPin = 2;    // 13 corresponds to GPIO13
const int greenPin = 0;  // 12 corresponds to GPIO12
const int bluePin = 3;   // 14 corresponds to GPIO14

void setup() {

  ArduinoOTA.onStart(update_started);
  ArduinoOTA.onProgress(update_progress);
  ArduinoOTA.onEnd(update_finished);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
  for(int i = 0; i < 10; i++)
  {
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    delay(30);
  }
  Serial.begin(115200);
  // Connect to Wi-Fi network with SSID and password
  WiFi.hostname("ESP Color Picker");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  DEBUG_MSG("----------Connected----------\n");
  ArduinoOTA.setHostname("ESP Color Picker");
  ArduinoOTA.begin();
  DEBUG_MSG("%s\n", WiFi.localIP().toString().c_str());
  server.begin();

  MDNS.begin("ESP-ColorPicker");
  MDNS.addService("http", "tcp", 80);

  analogWriteFreq(1000);
  analogWriteRange(1023);
  server.on("/rgb", handle_set_rgb);
  server.on("/fade", handle_fade_rgb);
}

void loop() {
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();
}

void rgb(int r, int g, int b, const uint32_t table[]){
  analogWrite(redPin, 0x3FF - table[r]);
  analogWrite(greenPin, 0x3FF - table[g]);
  analogWrite(bluePin, 0x3fF - table[b]);
  // DEBUG_MSG("[%d, %d, %d], [%d, %d, %d] \n", r, g, b, table[r], table[g], table[b]);
}

void handle_set_rgb(){
  StreamString temp;
  temp.reserve(1200);  // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("\
  <!DOCTYPE html><html>\
    <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
      <link rel=\"icon\" href=\"data:,\">\
      <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">\
      <script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>\
    </head>\
    <body>\
      <div class=\"container\">\
        <div class=\"row\">\
          <h1>ESP Color Picker</h1>\
        </div>\
        <a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a>\
        <input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\">\
      </div>\
      <script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);\
      document.getElementById(\"change_color\").href=\"?r=\" + Math.round(picker.rgb[0]) + \"&g=\" +  Math.round(picker.rgb[1]) + \"&b=\" + Math.round(picker.rgb[2]);}\
      </script>\
    </body>\
  </html>");
  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("g").toInt();
  rgb(r, g, b, cie);
  server.send(200, "text/html", temp.c_str());
  DEBUG_MSG("[%d, %d, %d] \n", r, g ,b);
}

void handle_fade_rgb(){
  const int steps = 100;
  int rs = server.arg("rs").toInt();
  int re = server.arg("re").toInt();
  int gs = server.arg("gs").toInt();
  int ge = server.arg("ge").toInt();
  int bs = server.arg("bs").toInt();
  int be = server.arg("be").toInt();
  int t = server.arg("t").toInt();
  DEBUG_MSG("[%d, %d, %d] to [%d, %d, %d] - %d ms\n", rs, gs, bs, re, ge, be, t);
  t = t*1000;
  if (t < 0)
    t = 0;
  uint64_t micros = micros64();
  for(int i = 0; i < steps; i++){
    uint64_t t_i = map(i, 0, steps - 1, t/steps , t);
    int r = map(i, 0, steps - 1, rs, re);
    int g = map(i, 0, steps - 1, gs, ge);
    int b = map(i, 0, steps - 1, bs, be);
    rgb(r,g,b, cie_100);
    uint64_t t_d = max(uint64_t (0), uint64_t(t_i - (micros64() - micros)));
    if (t_i > (micros64() - micros))
      delayMicroseconds(t_d);
    else
      t_d = 0;
    // DEBUG_MSG("%llu, %llu, %llu \n", t_i/1000, t_d/1000, (micros64() - micros)/1000);
  }
  
  server.send(200, "text/html", "");
}

void update_started() {
  DEBUG_MSG("----------OTA Update----------\n");
  DEBUG_MSG("Progress :\t0%%");
}

void update_progress(int progress, int total) {
  if (int(progress / (total / 100)) >= last_progress + 10)
  {
    last_progress += 10;
    DEBUG_MSG("\t%d%%", last_progress);
  }
}

void update_finished() {
  DEBUG_MSG("\n----------UPDATED----------\n");
}
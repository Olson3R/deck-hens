#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <sunset.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>

#define VERSION "0.3.2"

const char *rootCACertificate = "-----BEGIN CERTIFICATE-----\n"
"MIIEozCCBEmgAwIBAgIQTij3hrZsGjuULNLEDrdCpTAKBggqhkjOPQQDAjCBjzEL\n"
"MAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n"
"BxMHU2FsZm9yZDEYMBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTcwNQYDVQQDEy5T\n"
"ZWN0aWdvIEVDQyBEb21haW4gVmFsaWRhdGlvbiBTZWN1cmUgU2VydmVyIENBMB4X\n"
"DTI0MDMwNzAwMDAwMFoXDTI1MDMwNzIzNTk1OVowFTETMBEGA1UEAxMKZ2l0aHVi\n"
"LmNvbTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABARO/Ho9XdkY1qh9mAgjOUkW\n"
"mXTb05jgRulKciMVBuKB3ZHexvCdyoiCRHEMBfFXoZhWkQVMogNLo/lW215X3pGj\n"
"ggL+MIIC+jAfBgNVHSMEGDAWgBT2hQo7EYbhBH0Oqgss0u7MZHt7rjAdBgNVHQ4E\n"
"FgQUO2g/NDr1RzTK76ZOPZq9Xm56zJ8wDgYDVR0PAQH/BAQDAgeAMAwGA1UdEwEB\n"
"/wQCMAAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMEkGA1UdIARCMEAw\n"
"NAYLKwYBBAGyMQECAgcwJTAjBggrBgEFBQcCARYXaHR0cHM6Ly9zZWN0aWdvLmNv\n"
"bS9DUFMwCAYGZ4EMAQIBMIGEBggrBgEFBQcBAQR4MHYwTwYIKwYBBQUHMAKGQ2h0\n"
"dHA6Ly9jcnQuc2VjdGlnby5jb20vU2VjdGlnb0VDQ0RvbWFpblZhbGlkYXRpb25T\n"
"ZWN1cmVTZXJ2ZXJDQS5jcnQwIwYIKwYBBQUHMAGGF2h0dHA6Ly9vY3NwLnNlY3Rp\n"
"Z28uY29tMIIBgAYKKwYBBAHWeQIEAgSCAXAEggFsAWoAdwDPEVbu1S58r/OHW9lp\n"
"LpvpGnFnSrAX7KwB0lt3zsw7CAAAAY4WOvAZAAAEAwBIMEYCIQD7oNz/2oO8VGaW\n"
"WrqrsBQBzQH0hRhMLm11oeMpg1fNawIhAKWc0q7Z+mxDVYV/6ov7f/i0H/aAcHSC\n"
"Ii/QJcECraOpAHYAouMK5EXvva2bfjjtR2d3U9eCW4SU1yteGyzEuVCkR+cAAAGO\n"
"Fjrv+AAABAMARzBFAiEAyupEIVAMk0c8BVVpF0QbisfoEwy5xJQKQOe8EvMU4W8C\n"
"IGAIIuzjxBFlHpkqcsa7UZy24y/B6xZnktUw/Ne5q5hCAHcATnWjJ1yaEMM4W2zU\n"
"3z9S6x3w4I4bjWnAsfpksWKaOd8AAAGOFjrv9wAABAMASDBGAiEA+8OvQzpgRf31\n"
"uLBsCE8ktCUfvsiRT7zWSqeXliA09TUCIQDcB7Xn97aEDMBKXIbdm5KZ9GjvRyoF\n"
"9skD5/4GneoMWzAlBgNVHREEHjAcggpnaXRodWIuY29tgg53d3cuZ2l0aHViLmNv\n"
"bTAKBggqhkjOPQQDAgNIADBFAiEAru2McPr0eNwcWNuDEY0a/rGzXRfRrm+6XfZe\n"
"SzhYZewCIBq4TUEBCgapv7xvAtRKdVdi/b4m36Uyej1ggyJsiesA\n"
"-----END CERTIFICATE-----\n";

#define TIMEZONE_OFFSET_HOURS -5
#define LATITUDE              46.876914
#define LONGITUDE             -90.895601

const char* ntpServer = "pool.ntp.org";
const long  gmtOffsetSec = TIMEZONE_OFFSET_HOURS * 60 * 60;
const int   daylightOffsetSec = 0;

#define BUTTON    15
#define LEDSTRIP  2
#define NUMPIXELS 150 // pixels on RGB LED strip

#define EEPROM_SIZE 512
#define COLORSCALE 8 // 5 bit: 0-31 scaled to x8

Adafruit_AHTX0 aht;

IPAddress localIp(192, 168, 4, 1);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(255, 255, 255, 0);

const char* host = "deck-hens";
const char* update_username = "admin";
const char* update_password = "D3ckH3ns!";

AsyncWebServer httpServer(80);

const uint16_t MORNING_ON_MINS = 7 * 60;
const int16_t MORNING_OFF_OFFSET_MINS = 60;
const int16_t NIGHT_ON_OFFSET_MINS = -60;
const uint16_t NIGHT_OFF_MINS = 22 * 60;

void setupSchedule();
bool scheduleMorningOn(tm *timeinfo, bool fromSkip);
bool scheduleMorningOff(tm *timeinfo);
bool scheduleNightOn(tm *timeinfo, bool fromSkip);
bool scheduleNightOff(tm *timeinfo);
void morningOn();
void morningOff();
void nightOn();
void nightOff();

Ticker timerLight;
tm tickerTime = { 0 };
tm tickerSetTime = { 0 };
uint16_t tickerDelaySeconds = 0;
char tickerName[50];
char ahtStatus[50];
SunSet sun;

const unsigned long wifiTimeoutMs = 30 * 1000;
unsigned long lastWifiReconnectMs = 0;
unsigned long lastButtonPress = 0;
const int BUTTON_DELAY = 100;

bool upgradeSystem = false;

struct Led {
  uint32_t color;
  Adafruit_NeoPixel strip;
  char name[6];
};

Led led = { 0, Adafruit_NeoPixel(NUMPIXELS, LEDSTRIP, NEO_BRG + NEO_KHZ800), "HEN" };

struct {
  char wifiSsid[32] = "";
  char wifiPassword[32] = "";
} eepromStruct;

void getTime(tm *timeinfo) {
  time_t now = 0;
  time(&now);
  localtime_r(&now, timeinfo);
}

void changeLed(Led *led) {
  led->strip.fill(led->color);
  led->strip.setBrightness(255);
  led->strip.show();
}

void setupLed(Led *led) {
  changeLed(led);
}

void turnLedOff(Led *led) {
  led->color = led->strip.Color(0, 0, 0);
  changeLed(led);
}

void turnLedOn(Led *led) {
  led->color = led->strip.Color(255, 255, 255);
  changeLed(led);
}

void toggleLed(Led *led) {
  if (led->color > 0) {
    turnLedOff(led);
  } else {
    turnLedOn(led);
  }
}

float getTempF() {
  // sensors_event_t humidity, temp;
  // aht.getEvent(&humidity, &temp);
  // return temp.temperature * 9.0 / 5.0 + 32.0;
  // return (float) temp.temperature;
  return 2.34;
  // sensors.requestTemperatures();
  // return sensors.getTempFByIndex(0);
}

double getSunriseMins(tm *timeinfo) {
  sun.setCurrentDate(timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday);
  return sun.calcCivilSunrise();
}

double getSunsetMins(tm *timeinfo) {
  sun.setCurrentDate(timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday);
  return sun.calcCivilSunset();
}

String buildErrorJson(const char* message) {
  StaticJsonDocument<200> doc;
  JsonObject root = doc.to<JsonObject>();
  root["status"] = "ERROR";
  root["message"] = message;

  return doc.as<String>();
}

void handleFile(AsyncWebServerRequest *request, String path, String mimeType) {
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    char* contentType = "text/plain";
    AsyncWebServerResponse* response = request->beginResponse(SPIFFS, path, mimeType);
    response->addHeader("Cache-Control", "max-age=604800");
    request->send(response);
    file.close();
  }
  else {
    Serial.print(path);
    Serial.println(" not found!");
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  handleFile(request, "/index.html", "text/html");
}

void handleFavicon(AsyncWebServerRequest *request) {
  handleFile(request, "/favicon.ico", "image/x-icon");
}

void handlePreact(AsyncWebServerRequest *request) {
  handleFile(request, "/preact.js", "application/javascript");
}

void handlePreactHooks(AsyncWebServerRequest *request) {
  handleFile(request, "/preact-hooks.js", "application/javascript");
}

void handleHtm(AsyncWebServerRequest *request) {
  handleFile(request, "/htm.js", "application/javascript");
}

void handleStatus(AsyncWebServerRequest *request) {
  float tempF = getTempF();
  // float tempF = 1.22;
  // double sunsetOffsetMins = getSunsetOffsetMins();

  char hexColor[8];
  StaticJsonDocument<300> doc;
  JsonObject root = doc.to<JsonObject>();
  sprintf(hexColor, "#%06x", led.color);
  root["color"] = hexColor;
  root["tempF"] = tempF;
  char buf[50];
  root["nextEventName"] = tickerName;
  // byte ss = aht.getStatus();
  // sprintf(buf, "%b", ss);
  root["nextEventName"] = ahtStatus;
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S-0500", &tickerTime);
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S-0500", &tickerTime);
  root["nextEventTime"] = buf;
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S-0500", &tickerSetTime);
  root["nextEventSetTime"] = buf;
  root["nextEventDelaySeconds"] = tickerDelaySeconds;
  tm current;
  getTime(&current);
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S-0500", &current);
  root["currentTime"] = buf;
  root["version"] = VERSION;
  request->send(200, "application/json", doc.as<String>());
}

void handleBodyLed(AsyncWebServerRequest *request, uint8_t *data) {
  DynamicJsonDocument doc(200);
  DeserializationError error = deserializeJson(doc, (const char*) data);
  if (error) {
    String err = buildErrorJson(error.c_str());
    request->send(422, "application/json", err);
  }
  else {
    led.color = (uint32_t)strtol(&doc["color"].as<char*>()[1], NULL, 16);
    setupLed(&led);
    request->send(200, "application/json", "{\"status\":\"SUCCESS\"}");
  }
}

void handleBodyRestart(AsyncWebServerRequest *request, uint8_t *data) {
  request->send(200, "application/json", "{\"status\":\"SUCCESS\"}");
  delay(200);
  ESP.restart();
}

void handleBodyUpgradeSystem(AsyncWebServerRequest *request, uint8_t *data) {
  upgradeSystem = true;
  StaticJsonDocument<200> doc;
  JsonObject root = doc.to<JsonObject>();
  root["status"] = "SUCCESS";
  request->send(200, "application/json", doc.as<String>());
}

void handleBodyWifi(AsyncWebServerRequest *request, uint8_t *data) {
  DynamicJsonDocument doc(200);
  DeserializationError error = deserializeJson(doc, (const char*) data);
  if (error) {
    StaticJsonDocument<200> errDoc;
    JsonObject root = errDoc.to<JsonObject>();
    root["status"] = "ERROR";
    root["message"] = error.c_str();
    request->send(422, "application/json", errDoc.as<String>());
  }
  else {
    strcpy(eepromStruct.wifiSsid, doc["ssid"]);
    strcpy(eepromStruct.wifiPassword, doc["password"]);
    EEPROM.put(0, eepromStruct);
    EEPROM.commit();
    request->send(200, "application/json", "{\"status\":\"SUCCESS\"}");
    delay(2000);
    ESP.restart();
  }
}

void scheduleTimer(uint16_t delaySeconds, Ticker::callback_function_t callback, const char *name) {
  time_t now;
  time(&now);
  localtime_r(&now, &tickerSetTime);
  now = now + delaySeconds;
  localtime_r(&now, &tickerTime);
  strcpy(tickerName, name);
  tickerDelaySeconds = delaySeconds;

  Serial.print("Scheduling timer for ");
  Serial.print(delaySeconds);
  Serial.println(" seconds from now");
  char buf[50];
  strftime(buf, sizeof(buf), "%D %H:%M:%S", &tickerTime);
  Serial.println(buf);
  timerLight.once(delaySeconds, callback);
}

void morningOn() {
  led.color = led.strip.Color(255, 255, 255);
  changeLed(&led);

  tm timeinfo;
  getTime(&timeinfo);
  scheduleMorningOff(&timeinfo);
}

void morningOff() {
  led.color = led.strip.Color(255, 255, 255);
  changeLed(&led);

  tm timeinfo;
  getTime(&timeinfo);
  scheduleNightOn(&timeinfo, false);
}

void nightOn() {
  led.color = led.strip.Color(255, 255, 255);
  changeLed(&led);

  tm timeinfo;
  getTime(&timeinfo);
  scheduleNightOff(&timeinfo);
}

void nightOff() {
  led.color = led.strip.Color(0, 0, 0);
  changeLed(&led);

  tm timeinfo;
  getTime(&timeinfo);
  scheduleMorningOn(&timeinfo, false);
}

uint16_t getDayMins(tm *timeinfo) {
  return timeinfo->tm_hour * 60 + timeinfo->tm_min;
}

bool scheduleNightOff(tm *timeinfo) {
  Serial.println("Schedule night off");
  uint16_t dayMins = getDayMins(timeinfo);
  int sunsetMins = (int) getSunsetMins(timeinfo);
  int timerMins = 0;

  if (dayMins < NIGHT_OFF_MINS) {
    timerMins = NIGHT_OFF_MINS - dayMins;
  }

  if (timerMins > 0) {
    scheduleTimer(timerMins * 60, nightOff, "Night Off");
    return true;
  }

  return false;

}

bool scheduleNightOn(tm *timeinfo, bool fromSkip) {
  Serial.println("Schedule night on");
  uint16_t dayMins = getDayMins(timeinfo);
  uint16_t sunsetMins = (uint16_t) getSunsetMins(timeinfo);
  int timerMins = 0;

  if ((sunsetMins + NIGHT_ON_OFFSET_MINS) > NIGHT_OFF_MINS) {
    if (fromSkip) {
      scheduleTimer((1440 - dayMins + 10) * 60, setupSchedule, "Setup Schedule");
      return false;
    }

    return scheduleMorningOn(timeinfo, true);
  }

  // TODO: Minor improvement to get the sunrise for the next day
  if (dayMins < (sunsetMins + NIGHT_ON_OFFSET_MINS)) {
    timerMins = sunsetMins + NIGHT_ON_OFFSET_MINS - dayMins;
  } else if (fromSkip && dayMins >= NIGHT_OFF_MINS) {
    timerMins = (1440 - dayMins) + sunsetMins + NIGHT_ON_OFFSET_MINS;
  }

  if (timerMins > 0) {
    scheduleTimer(timerMins * 60, nightOn, "Night On");
    return true;
  }

  if (dayMins < NIGHT_OFF_MINS) {
    nightOn();
    return true;
  }

  return false;
}

bool scheduleMorningOff(tm *timeinfo) {
  Serial.println("Schedule morning off");
  uint16_t dayMins = getDayMins(timeinfo);
  int sunriseMins = (int) getSunriseMins(timeinfo);
  int timerMins = 0;

  if (dayMins < (sunriseMins + MORNING_OFF_OFFSET_MINS)) {
    timerMins = sunriseMins + MORNING_OFF_OFFSET_MINS - dayMins;
  }

  if (timerMins > 0) {
    scheduleTimer(timerMins * 60, morningOff, "Morning Off");
    return true;
  }

  return false;
}

bool scheduleMorningOn(tm *timeinfo, bool fromSkip) {
  Serial.println("Schedule morning on");
  uint16_t dayMins = getDayMins(timeinfo);
  int sunriseMins = (int) getSunriseMins(timeinfo);
  int timerMins = 0;

  if ((sunriseMins + MORNING_OFF_OFFSET_MINS) < MORNING_ON_MINS) {
    if (fromSkip) {
      scheduleTimer((1440 - dayMins + 10) * 60, setupSchedule, "Setup Schedule");
      return false;
    }

    return scheduleNightOn(timeinfo, true);
  }

  // TODO: Minor improvement to get the sunrise for the next day
  if (dayMins >= NIGHT_OFF_MINS) {
    timerMins = 1440 - dayMins + MORNING_ON_MINS;
  } else if (dayMins < MORNING_ON_MINS) {
    timerMins = MORNING_ON_MINS - dayMins;
  } else if (fromSkip && dayMins >= (sunriseMins + MORNING_OFF_OFFSET_MINS)) {
    timerMins = (1440 - dayMins) + MORNING_ON_MINS;
  }

  // schedule on
  if (timerMins > 0) {
    scheduleTimer(timerMins * 60, morningOn, "Morning On");
    return true;
  }

  // turn on now
  if (dayMins < (sunriseMins + MORNING_OFF_OFFSET_MINS)) {
    morningOn();
    return true;
  }

  return false;
}

void setupSchedule() {
  Serial.println("Setup schedule");

  tm timeinfo;
  getTime(&timeinfo);

  if (scheduleMorningOn(&timeinfo, false)) return;
  if (scheduleMorningOff(&timeinfo)) return;
  if (scheduleNightOn(&timeinfo, false)) return;
  scheduleNightOff(&timeinfo);
}

void startSoftAp() {
  WiFi.softAPConfig(localIp, gateway, subnet);
  WiFi.softAP(host, "12345678");
  Serial.println("Soft AP: ");
  Serial.println(WiFi.softAPIP());
}

bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void checkWifi() {
  unsigned long now = millis();
  if (!wifiConnected() && (now - lastWifiReconnectMs) >= wifiTimeoutMs) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    lastWifiReconnectMs = now;
  }
}

void ICACHE_RAM_ATTR ISR() {
  if (millis() - lastButtonPress > BUTTON_DELAY) {
      toggleLed(&led);
      lastButtonPress = millis();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Starting LED Strips...");
  led.strip.begin(); // This initializes the NeoPixel library.
  led.strip.clear();
  led.strip.show();

  Serial.println("Starting Temp Sensor...");
  if (aht.begin()) {
    Serial.println("Found AHT20");
    strcpy(ahtStatus, "FOUND");
  } else {
    Serial.println("Didn't find AHT20");
    strcpy(ahtStatus, "MISSING");
  }

  Serial.println("Starting EEPROM...");
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, eepromStruct);

  Serial.println("Starting WiFi...");
  Serial.print("SSID: ");
  Serial.println(eepromStruct.wifiSsid);

  WiFi.mode(WIFI_AP_STA);
  if (eepromStruct.wifiSsid && *eepromStruct.wifiSsid != 0x00 && strlen(eepromStruct.wifiSsid) > 0 && strlen(eepromStruct.wifiSsid) <= 32) {
    WiFi.begin(eepromStruct.wifiSsid, eepromStruct.wifiPassword);

    int wifiAttempts = 0;
    int maxWifiAttempts = 1;
    while (WiFi.waitForConnectResult() != WL_CONNECTED && wifiAttempts < maxWifiAttempts) {
      WiFi.begin(eepromStruct.wifiSsid, eepromStruct.wifiPassword);
      wifiAttempts++;

      if (wifiAttempts < maxWifiAttempts) {
        Serial.println("WiFi failed, retrying...");
      }
      else {
        startSoftAp();
      }
    }
  }
  else {
    startSoftAp();
  }

  Serial.println("Configuring Time...");
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  sun.setPosition(LATITUDE, LONGITUDE, TIMEZONE_OFFSET_HOURS);

  Serial.println("Configuring SPIFFS...");
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Serial.println("Configuring Server & OTA...");
  ElegantOTA.begin(&httpServer, update_username, update_password);
  httpServer.on("/", handleRoot);
  httpServer.on("/favicon.ico", handleFavicon);
  httpServer.on("/preact.js", handlePreact);
  httpServer.on("/preact-hooks.js", handlePreactHooks);
  httpServer.on("/htm.js", handleHtm);
  httpServer.on("/status", HTTP_GET, handleStatus);
  httpServer.onRequestBody(
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (request->url() == "/led") {
        return handleBodyLed(request, data);
      }
      if (request->url() == "/restart") {
        return handleBodyRestart(request, data);
      }
      if (request->url() == "/upgrade-system") {
        return handleBodyUpgradeSystem(request, data);
      }
      if (request->url() == "/wifi") {
        return handleBodyWifi(request, data);
      }
    }
  );

  httpServer.begin();

  Serial.println("Configuring MDNS...");
  MDNS.begin(host);
  MDNS.addService("http", "tcp", 80);
  Serial.printf("Server ready! Open http://%s.local in your browser\n", host);

  Serial.println("Setting schedules...");
  setupSchedule();

  Serial.println("Setup button...");
  attachInterrupt(BUTTON, ISR, RISING);

  Serial.println("Booted!");
}

void loop() {
  checkWifi();
  if (!wifiConnected()) {
    Serial.println("No Wifi");
  }

  MDNS.update();

  if (upgradeSystem) {
    upgradeSystem = false;
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    Serial.println("Update FS...");
    // NetworkClientSecure client;
    // client.setCACert(rootCACertificate);
    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    // Reading data over SSL may be slow, use an adequate timeout
    client.setTimeout(12000);  // timeout argument is defined in milliseconds for setTimeout

    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    t_httpUpdate_return ret = ESPhttpUpdate.updateFS(client, "https://raw.githubusercontent.com/Olson3R/deck-hens/main/release/latest/spiffs.bin");
    if (ret == HTTP_UPDATE_OK) {
      Serial.println("Update sketch...");
      // ret = ESPhttpUpdate.update(client, "https://raw.githubusercontent.com/Olson3R/deck-hens/main/release/latest/firmware.bin");
      // if (ret == HTTP_UPDATE_OK) {
        StaticJsonDocument<200> doc;
        JsonObject root = doc.to<JsonObject>();
        delay(2000);
        ESP.restart();
      // }
    } else {
      strcpy(ahtStatus, ESPhttpUpdate.getLastErrorString().c_str());
    }
  }

  float temperatureF = getTempF();

  Serial.print("T: ");
  Serial.println(temperatureF);

  delay(5000); // 5 seconds
}

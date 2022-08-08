
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Servo.h>

Servo motor;

bool enabled = false, repeat = false;

//const char *ssid = "E-Seabike";
//int motorSpeeds[5] = {0, 0, 0, 0, 0};
//byte numberOfSpeeds = 0;
//ESP8266WebServer HTTP(80);
//
//void showConfigFile() {
//  String cfgRead;
//  File cfg = SPIFFS.open("/config.txt", "r");
//  if (!cfg) Serial.println("Error opening file");
//  cfgRead = cfg.readString();  //speeds:3;speed_1:1000;speed_2:1100;speed_3:1200;speed_4:1300;speed_5:1400;
//  Serial.println("Config file:" + cfgRead);
//  cfg.close();
//};
//
//void setConfigFromFlash() {
//  String configStream[6] = {"", "", "", "", "", ""};
//  String configStreamingValues[6] = {"speeds", "speed_0", "speed_1", "speed_2", "speed_3", "speed_4"};
//  String cfgRead;
//
//  File cfg = SPIFFS.open("/config.txt", "r");
//  if (!cfg) Serial.println("Error opening file");
//  cfgRead = cfg.readString();  //speeds:3;speed_1:1000;speed_2:1100;speed_3:1200;speed_4:1300;speed_5:1400;
//  cfg.close();
//
//  int i;
//  for (i = 0; i < 6; i++) {
//    if (i < 5) configStream[i] = cfgRead.substring(cfgRead.indexOf(configStreamingValues[i]) + configStreamingValues[i].length() + 1, cfgRead.indexOf(configStreamingValues[i + 1]) - 1);
//    if (i == 5) configStream[i] = cfgRead.substring(cfgRead.indexOf(configStreamingValues[i]) + configStreamingValues[i].length() + 1, cfgRead.length() - 3);
//  };
//
//  numberOfSpeeds = configStream[0].toInt();
//  motorSpeeds[0] = configStream[1].toInt();
//  motorSpeeds[1] = configStream[2].toInt();
//  motorSpeeds[2] = configStream[3].toInt();
//  motorSpeeds[3] = configStream[4].toInt();
//  motorSpeeds[4] = configStream[5].toInt();
//
//  Serial.println("Update config from FLASH!");
//  printConfig();
//};
//
//void printConfig() {
//  int i;
//  Serial.println("Number of speeds: " + (String)numberOfSpeeds);
//  for (i = 0; i < 5; i++) Serial.println("Speed " + (String)i + ": " + (String)motorSpeeds[i]);
//};
//
//void sendDeviceParams() {
//  String resMessage = "";
//  int i;
//
//  resMessage += " speeds: ";
//  resMessage += (String)numberOfSpeeds;
//  resMessage += "; ";
//
//  for (i = 0; i < 5; i++) {
//    resMessage += "speed ";
//    resMessage += (String)i;
//    resMessage += ": ";
//    resMessage += (String)motorSpeeds[i];
//    resMessage += "; ";
//  };
//
//  HTTP.send(200, "text/plain", resMessage);
//
//  Serial.println("Send responce params http!");
//};
//
//void setConfigFromHttp() {
//
//  numberOfSpeeds = HTTP.arg("speeds").toInt();
//  motorSpeeds[0] = HTTP.arg("speed_0").toInt();
//  motorSpeeds[1] = HTTP.arg("speed_1").toInt();
//  motorSpeeds[2] = HTTP.arg("speed_2").toInt();
//  motorSpeeds[3] = HTTP.arg("speed_3").toInt();
//  motorSpeeds[4] = HTTP.arg("speed_4").toInt();
//
//  saveConfigFile();
//  sendDeviceParams();
//};
//
//void saveConfigFile() {
//  String writeConfig = "";
//  int i;
//
//  writeConfig += " speeds:";
//  writeConfig += (String)numberOfSpeeds;
//  writeConfig += ";";
//  for (i = 0; i < 5; i++) {
//    writeConfig += "speed_";
//    writeConfig += (String)i;
//    writeConfig += ":";
//    writeConfig += (String)motorSpeeds[i];
//    writeConfig += ";";
//  };
//
//  File cfgWrite = SPIFFS.open("/config.txt", "w");
//  if (!cfgWrite) Serial.println("Error opening file");
//  cfgWrite.println(writeConfig);
//  cfgWrite.close();
//
//  Serial.println("Wriete config file:" + writeConfig);
//};

void setup() {

  Serial.begin(9600);
  motor.attach(15);
  motor.writeMicroseconds(800);
  delay(5000);
  //  motor.writeMicroseconds(1500);

  pinMode(14, INPUT_PULLUP);



  //  WiFi.softAP(ssid);
  //
  //  bool success = SPIFFS.begin();
  //  if (!success) Serial.print("File system error!");
  //
  //
  //  //printConfig();
  //  //showConfigFile();
  //  setConfigFromFlash();
  //
  //  HTTP.begin();
  //
  //  Serial.print("My IP: ");
  //  Serial.println(WiFi.softAPIP());
  //
  //  HTTP.on("/status", sendDeviceParams);
  //
  //  HTTP.on("/speed", setConfigFromHttp);

}

void loop() {
  //  HTTP.handleClient();

  static uint32_t tmr;
  if (millis() - tmr >= 300) {
    tmr = millis();


    if (!digitalRead(14)) {
      if (!repeat) enabled = !enabled;
      repeat = true;
    } else {
      repeat = false;
    }

    if (enabled) {
      motor.writeMicroseconds(1500);
    } else {
      motor.writeMicroseconds(800);
    };

    Serial.println("XYQ");

  }


}

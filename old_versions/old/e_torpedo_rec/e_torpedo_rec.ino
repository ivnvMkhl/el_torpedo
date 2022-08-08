#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
//#include <EncButton.h>
//EncButton<EB_TICK, 2> btn(INPUT_PULLUP);

float Yaw, Roll, Pitch, magx, magy, magz, accx, accy, accz, gyrox, gyroy, gyroz, q0, q1, q2, q3, Roll2, Pitch2, Yaw2, LIAx, LIAy, LIAz, GRVx, GRVy, GRVz;
float LastYaw = 0;
float YawSpread = 0;
File LoggerCommon;
File LoggerCommonRel;
String LogFileName = "tele.txt";
String LogFileNameRel = "tele_rel.txt";
unsigned long TimeSpread = 0;
unsigned long TimeSpreadRel = 0;
float SpreadResult = 0;
float SpreadResultRel = 0;
bool FixMove = false;

const int GY_955 = 0x29;
void setup()
{
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  
  Wire.begin();
  Wire.setClock(400000); // I2C clock rate ,You can delete it but it helps the speed of I2C (default rate is 100000 Hz)
  delay(100);
  Wire.beginTransmission(GY_955);
  Wire.write(0x3E); // Power Mode
  Wire.write(0x01); // Normal:0X00 (or B00), Low Power: 0X01 (or B01) , Suspend Mode: 0X02 (orB10)
  Wire.endTransmission();
  delay(100);
  Wire.beginTransmission(GY_955);
  Wire.write(0x3D); // Operation Mode
  Wire.write(0x0C); //NDOF:0X0C (or B1100) , IMU:0x08 (or B1000) , NDOF_FMC_OFF: 0x0B (or B1011)
  Wire.endTransmission();
  delay(100);
  Serial.begin(9600);  //Setting the baudrate
  delay(100);

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  /* tmElements_t tm;
    if (RTC.read(tm)) {
     LogFileName = String(String(tm.Month, DEC) + "_" + String(tm.Day, DEC) + "_" + String(tm.Hour, DEC) + "." + String(tm.Minute, DEC));
    }*/

  LoggerCommon = SD.open(LogFileName, FILE_WRITE);
  if (LoggerCommon)
  {
    LoggerCommon.println("Starting...");
    LoggerCommon.close();
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening telemetry file");
  }

  LoggerCommonRel = SD.open(LogFileNameRel, FILE_WRITE);
  if (LoggerCommonRel)
  {
    LoggerCommonRel.println("Starting...");
    LoggerCommonRel.close();
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening telemetry abs file");
  }
}
void loop()
{
  //замеры уровней напряжения и токов
  int current_in = analogRead(A1);
  int current = map(current_in, 510, 1020, 0, 255);
  int voltage_in = analogRead(A0);
  int voltage1 = map(voltage_in, 407, 792, 1137, 2199);
  int voltage = map(voltage1, 2436, 2725, 2185, 2450);

  static bool enabled = false;
  static bool enabledflag = false;
  static bool renamefile = true;
  static unsigned long lineN = 0;

  enabled = !digitalRead(2); //получение флага включения на 2 вход

  if (enabled != enabledflag)
  {
    enabledflag = enabled;
    renamefile = true;
  }


  if (renamefile) {
    tmElements_t tm;
    if (RTC.read(tm)) {
      LogFileName = String(String(tm.Month, DEC) + "_" + String(tm.Day, DEC) + "_" + String(tm.Hour, DEC) + "." + String(tm.Minute, DEC));
      Serial.println(renamefile);
    }
    Serial.println("###########################################################################################");
    LoggerCommon = SD.open(LogFileName, FILE_WRITE);
    if (LoggerCommon) {
      LoggerCommon.println("###########################################################################################");
      LoggerCommon.close();
    }
    LoggerCommonRel = SD.open(LogFileNameRel, FILE_WRITE);
    if (LoggerCommonRel)
    {
      LoggerCommon.println("###########################################################################################");
      LoggerCommonRel.close();
    }
    renamefile = false;
    lineN = 0;
  }

  Wire.beginTransmission(GY_955);
  Wire.write(0x08);
  Wire.endTransmission(false);
  Wire.requestFrom(GY_955, 32, true);
  // Accelerometer
  accx = (int16_t)(Wire.read() | Wire.read() << 8 ) / 100.00; // m/s^2
  accy = (int16_t)(Wire.read() | Wire.read() << 8 ) / 100.00; // m/s^2
  accz = (int16_t)(Wire.read() | Wire.read() << 8 ) / 100.00; // m/s^2
  // Magnetometer
  magx = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; // mT
  magy = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; // mT
  magz = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; // mT
  // Gyroscope
  gyrox = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; // Dps
  gyroy = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; // Dps
  gyroz = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; // Dps
  // Euler Angles
  Yaw = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; //in Degrees unit
  Roll = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; //in Degrees unit
  Pitch = (int16_t)(Wire.read() | Wire.read() << 8 ) / 16.00; //in Degrees unit
  // Quaternions
  q0 = (int16_t)(Wire.read() | Wire.read() << 8 ) / (pow(2, 14)); //unit less
  q1 = (int16_t)(Wire.read() | Wire.read() << 8 ) / (pow(2, 14)); //unit less
  q2 = (int16_t)(Wire.read() | Wire.read() << 8 ) / (pow(2, 14)); //unit less
  q3 = (int16_t)(Wire.read() | Wire.read() << 8 ) / (pow(2, 14)); //unit less
  //Convert Quaternions to Euler Angles
  Yaw2 = (atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (pow(q2, 2) + pow(q3, 2)))) * 180 / PI;
  Roll2 = (asin(2 * (q0 * q2 - q3 * q1))) * 180 / PI;
  Pitch2 = (atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (pow(q1, 2) + pow(q2, 2)))) * 180 / PI;
  //Linear (Dynamic) & Gravitational (static) Acceleration
  Wire.beginTransmission(0x29);
  Wire.write(0x28);
  Wire.endTransmission(false);
  Wire.requestFrom(0x29, 12, true);
  LIAx = (int16_t)(Wire.read() | Wire.read() << 8) / 100.00; // m/s^2
  LIAy = (int16_t)(Wire.read() | Wire.read() << 8) / 100.00; // m/s^2
  LIAz = (int16_t)(Wire.read() | Wire.read() << 8) / 100.00; // m/s^2
  GRVx = (int16_t)(Wire.read() | Wire.read() << 8) / 100.00; // m/s^2
  GRVy = (int16_t)(Wire.read() | Wire.read() << 8) / 100.00; // m/s^2
  GRVz = (int16_t)(Wire.read() | Wire.read() << 8) / 100.00; // m/s^2
  Yaw = Yaw2;
  float newYawSpread = 0;
  if (LastYaw != 0)
  {
    if (LastYaw < 90 && Yaw > 270)  // 360 --> 1, left drift, - value
    {
      newYawSpread = - (LastYaw + 360 - Yaw);
    }
    else if (Yaw < 90 && LastYaw > 270) // 1 --> 360, right drift, + value
    {
      newYawSpread = 360 - LastYaw + Yaw;
    }
    else
    {
      newYawSpread = Yaw - LastYaw;
    }
  }

  SpreadResult += newYawSpread;
  SpreadResultRel += newYawSpread;
  unsigned long now = millis();
  if (newYawSpread > 0 && YawSpread < 0) // <-|
  {
    Serial.println(" <-|");
    //SpreadResultRel = 0;
    if (TimeSpreadRel > 0)
      TimeSpreadRel = now - TimeSpreadRel;
    FixMove = true;
  }
  if (newYawSpread < 0 && YawSpread > 0) // |->
  {
    Serial.println("|-> ");

    if (TimeSpreadRel > 0)
      TimeSpreadRel = now - TimeSpreadRel;
    FixMove = true;
  }

  if (TimeSpread > 0)
    TimeSpread = now - TimeSpread;
  /*tmElements_t tm;
    if (RTC.read(tm)) {
    String content = String(tmYearToCalendar(tm.Year), DEC) + "-" + String(tm.Month, DEC) + "-" + String(tm.Day, DEC) + " " + String(tm.Hour, DEC) + ":" + String(tm.Minute, DEC) + ":" + String(tm.Second, DEC);
        Serial.print(content);
        Serial.print(";");
        Serial.print(millis());
        Serial.print(";");
        Serial.print(SpreadResult);
        Serial.print(";");
        Serial.print(TimeSpread);
        Serial.print(";");
        Serial.print(SpreadResult / TimeSpread);
        Serial.print(";");
        Serial.println(Yaw);
    }*/

  if (enabled/* && FixMove*/)
  {
    LoggerCommon = SD.open(LogFileName, FILE_WRITE);
    if (LoggerCommon) {
      LoggerCommon.print(lineN);
      LoggerCommon.print("\t");
      LoggerCommon.print(millis());
      LoggerCommon.print("\t");
      LoggerCommon.print(current);
      LoggerCommon.print("\t");
      LoggerCommon.print(voltage);
      LoggerCommon.print("\t");
      LoggerCommon.print(accx);
      LoggerCommon.print("\t");
      LoggerCommon.print(gyrox);
      LoggerCommon.print("\t");
      LoggerCommon.print(SpreadResult);
      LoggerCommon.print("\t");
      LoggerCommon.print(Yaw);
      LoggerCommon.print("\t");
      LoggerCommon.println(TimeSpread);

      Serial.print("abs");
      Serial.print("\t");
      Serial.print(lineN);
      Serial.print("\t");
      Serial.print(millis());
      Serial.print("\t");
      Serial.print(current);
      Serial.print("\t");
      Serial.print(voltage);
      Serial.print("\t");
      Serial.print(accx);
      Serial.print("\t");
      Serial.print(gyrox);
      Serial.print("\t");
      Serial.print(SpreadResult);
      Serial.print("\t");
      Serial.print(TimeSpread);
      Serial.print("\t");
      Serial.println(Yaw);
    }

    TimeSpread = now;
    LoggerCommon.close();
  }

  if (enabled && FixMove)
  {
    LoggerCommonRel = SD.open(LogFileNameRel, FILE_WRITE);
    if (LoggerCommonRel) {
      LoggerCommonRel.print(lineN);
      LoggerCommonRel.print("\t");
      LoggerCommonRel.print(millis());
      LoggerCommonRel.print("\t");
      LoggerCommonRel.print(SpreadResultRel);
      LoggerCommonRel.print("\t");
      LoggerCommonRel.print(TimeSpreadRel);
      LoggerCommonRel.print("\t");
      LoggerCommonRel.println(Yaw);

      Serial.print("rel");
      Serial.print("\t");
      Serial.print(lineN);
      Serial.print("\t");
      Serial.print(millis());
      Serial.print("\t");
      Serial.print(SpreadResultRel);
      Serial.print("\t");
      Serial.print(TimeSpreadRel);
      Serial.print("\t");
      Serial.println(Yaw);
      LoggerCommonRel.close();
    }
    TimeSpreadRel = now;
    SpreadResultRel = 0;
    FixMove = false;
  }

  if (newYawSpread != 0)
    YawSpread = newYawSpread;

  LastYaw = Yaw;
  /*Serial.print("Yaw2=");
    Serial.print(Yaw2);
    Serial.print(" Roll2=");
    Serial.print(Roll2);
    Serial.print(" Pitch2=");
    Serial.println(Pitch2);*/
  lineN++;
  delay(50);
}

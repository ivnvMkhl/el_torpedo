#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
//#include <cppQueue.h>
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

//#define  IMPLEMENTATION  FIFO
//#define OVERWRITE   true
//cppQueue accValues((sizeof(float), 7, IMPLEMENTATION, OVERWRITE));
float accValues[7];
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

  Serial.println("############################");
  /* tmElements_t tm;
    if (RTC.read(tm)) {
     LogFileName = String(String(tm.Month, DEC) + "_" + String(tm.Day, DEC) + "_" + String(tm.Hour, DEC) + "." + String(tm.Minute, DEC));
    }*/
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
  static float threshold = 0.8;
  static float candidateMinMax = 0;
  static float lastMinMax = 0;
  static float spreadMinMax = 0;
  static bool motorRun = false;
  static int counterSec = 0;
  static int counterMinMax = 0;

  enabled = !digitalRead(2); //получение флага включения на 2 вход

  if (enabled != enabledflag)
  {
    enabledflag = enabled;
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

  //if (accValues.isFull())
  //  accValues.pop();
  //Serial.print("==========================");
  for (int i = 6; i > 0; i--)
  {
    /*Serial.print(accValues[i + 1]);
      Serial.print(" ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(accValues[i]);*/
    accValues[i] = accValues[i - 1];
  }

  int frezCount = 0;
  for (int i = 6; i > 0; i--)
  {
    /*Serial.print(accValues[i + 1]);
      Serial.print(" ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(accValues[i]);*/
    if (accValues[i] == accValues[i - 1])
      frezCount++;
  }

  if (frezCount == 5)
    Serial.println("-------------------- zzzzzzzzzzzzzzzzzzzzzz ------------------------");

  accValues[0] = accx;
  Serial.println(accx);
  float x0 = (accValues[0] + accValues[1] + accValues[2] + accValues[3] + accValues[4]) / 5;
  float x1 = (accValues[1] + accValues[2] + accValues[3] + accValues[4] + accValues[5]) / 5;
  float x2 = (accValues[2] + accValues[3] + accValues[4] + accValues[5] + accValues[6]) / 5;
  float value = x1;
  /*Serial.print(accValues[0]);
    Serial.print(accValues[1]);
    Serial.print(accValues[2]);
    Serial.print(accValues[3]);
    Serial.print(accValues[4]);
    Serial.print(accValues[5]);
    Serial.println(accValues[6]);
    Serial.print(x0);
    Serial.print(x1);
    Serial.println(x2);*/
  bool isMax = false;
  bool isMin = false;
  if (x0 > value && x2 > value)
    isMin = true;
  if (x0 < value && x2 < value)
    isMax = true;
  if (isMax || isMin)
  {
    //Serial.println("MinMax");
    if (abs(candidateMinMax - value) > threshold)
    {
      if (abs(candidateMinMax - lastMinMax) > threshold)
      {
        float spreadMinMaxPrev = spreadMinMax;
        float spreadMinMax = 0;
        if (candidateMinMax > lastMinMax)
          spreadMinMax = candidateMinMax - (candidateMinMax - lastMinMax) / 2;
        else
          spreadMinMax = lastMinMax - (lastMinMax - candidateMinMax) / 2;
        Serial.print("#####################################");
        Serial.print(lastMinMax);
        Serial.print(" ");
        Serial.println(candidateMinMax);
        counterMinMax++;
        /*if (!motorRun && abs(spreadMinMaxPrev - spreadMinMax) < threshold)
        {
          motorRun = true;
          
        }*/
        // meanSpread = (spreadMinMaxPrev + spreadMinMax) / 2;
        /*if (motorRun && ((isMax and value < spreadMinMax) || (isMin and value > spreadMinMax)))
        {
          motorRun = false;
          Serial.println("turnoff");
        }*/
        lastMinMax = candidateMinMax;
      }
      candidateMinMax = value;
    }
    else
    {
      candidateMinMax = lastMinMax;
    }
  }

  if (counterSec > 20)
  {
    if (counterMinMax > 0 && !motorRun)
    {
      motorRun = true;
      Serial.println("turn on motor");
      digitalWrite(5, LOW);
    }
    if (counterMinMax == 0 && motorRun)
    {
      motorRun = false;
      digitalWrite(5, HIGH);
      Serial.println("turn off motor");
    }
    counterSec = 0;
    counterMinMax = 0;
  }
  
  counterSec ++;
  lineN++;
  delay(50);
}

#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
//#include <cppQueue.h>
//#include <EncButton.h>
//EncButton<EB_TICK, 2> btn(INPUT_PULLUP);

//float Yaw, Roll, Pitch, magx, magy, magz, accx, accy, accz, gyrox, gyroy, gyroz, q0, q1, q2, q3, Roll2, Pitch2, Yaw2, LIAx, LIAy, LIAz, GRVx, GRVy, GRVz;
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
const int MPU = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
void setup()
{
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);

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
  static float threshold = 0.9;
  static float motorThreshholdCoeff = 1;
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

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
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

  accValues[0] = GyY / 1000.0;
  //Serial.println(AcY / 1000.0);
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
      if (abs(candidateMinMax - lastMinMax) > threshold * motorThreshholdCoeff)
      {
        float spreadMinMaxPrev = spreadMinMax;
        float spreadMinMax = 0;
        if (candidateMinMax > lastMinMax)
          spreadMinMax = candidateMinMax - (candidateMinMax - lastMinMax) / 2;
        else
          spreadMinMax = lastMinMax - (lastMinMax - candidateMinMax) / 2;
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


  if (counterSec < 20)
  {
    if (!motorRun)
    {
      motorThreshholdCoeff = 1;

      if (counterMinMax > 0)
      {
        motorRun = true;
        Serial.println("turn on motor");
        digitalWrite(5, LOW);
      }

    }
    else
    {
      motorThreshholdCoeff = 1;
      if (counterMinMax == 0)
      {
        motorRun = false;
        digitalWrite(5, HIGH);
        Serial.println("turn off motor");
      }
    }
  }
  if (counterSec > 20)
  {
    counterSec = 0;
    counterMinMax = 0;
  }

  counterSec ++;
  lineN++;
  delay(50);
}

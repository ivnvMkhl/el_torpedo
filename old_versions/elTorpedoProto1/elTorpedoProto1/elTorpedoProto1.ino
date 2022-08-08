/*
  pinOUT
  2 - КНОПКА
  3 - СД ЗЕЛЕНЫЙ
  4 - Сигнал зарядки +
  5 - СД КРАСНЫЙ
  6 - ДАТЧИК ХОЛЛА 1
  7 - МОТОР
  8 - ДАТЧИК ХОЛЛА 2
  9 - 
  10 - СД СИНИЙ
  11 - ВКЛ ЗАПИСИ ВЫХОД
  12 - ВКЛ МОТОРА ВХОД
  13 - ВКЛ МОТОРА ВХОД 2
  А0 - уровень напряжения
*/

#include <EncButton.h>
EncButton<EB_TICK, 2> btn(INPUT_PULLUP);
#include <Servo.h>
Servo motor;

const int numReading = 3;
uint32_t average = 0, d1 = 50000, d2 = 25000, d3 = 15000, values[numReading];
int p1 = 1550, p2 = 1800, p3 = 2200, powerMotor, index = 0;
double koeff = 19000.0;

void setup() {
  Serial.begin(9600);
  pinMode(10, OUTPUT); //Blue
  pinMode(3, OUTPUT); //Green
  pinMode(5, OUTPUT); //RED
  pinMode(11, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  digitalWrite(10, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(3, HIGH);
  pinMode(6, INPUT);
  pinMode(8, INPUT);
  motor.attach(7);


  motor.writeMicroseconds(800);
  delay(3000); //задержка при включении для инициализации ESC
  
  for (int i = 0; i < numReading; i++)
  {
    values[i] = 0;
  }

}

void loop() {
  static bool enabled, sensHall1, sensHall2, startHall1, startHall2;
  static bool specModeEnabled = false;
  static bool motorEnabled = false;
  static bool blickFlag = false;
  static int blickTimer = 0;
  static int specModeClickTimer = 0;
  static int motorStopVal = 950;
  static int motorGoVal = 1550;
  static int cycle = 5000;
  static int numTriggeredH = -1;
  int chargeLvl = analogRead(A0);
  static int chargeLvlR;
  static int chargeLvlG;
  static int h2DurTime;
  static int h1DurTime;
  static uint64_t h1Micros = 0;
  static uint64_t lastTriggerHallMicros;
  static uint32_t specModeTimer = 0;
  static uint32_t lastTriggerHall;
  
  
  // ремап цветовой индикации
  chargeLvl = map(chargeLvl, 0, 1023, 630, 840);
  chargeLvlR = map(chargeLvl, 750, 630, 253, 0);
  chargeLvlR = constrain(chargeLvlR, 0, 253);
  chargeLvlG = map(chargeLvl, 700, 840, 253, 0);
  chargeLvlG = constrain(chargeLvlG, 0, 253);

  //каждый луп смотрю состояние датчиков холла
  sensHall1 = !digitalRead(6);
  sensHall2 = !digitalRead(8);
  bool specMotor = !digitalRead(12);
 // Serial.println(specMotor);

  //при нажатии кнопки меняется состояние флага включения + включение спец режима
  btn.tick();
  if (btn.isClick()) {

    specModeEnabled = false;
    
    if (specModeEnabled)
    {
      specModeTimer = 0;
      specModeClickTimer = 0;
    }
    specModeClickTimer++;
    
    enabled = !enabled;
    
    if (specModeTimer == 0)
      specModeTimer = millis();
    
    if (millis() - specModeTimer < 5000 && specModeClickTimer == 6)
    {
      Serial.println("------------------ specModeEnabled ------------------");
      specModeEnabled = true;
      enabled = true;
    }
    
    if (specModeTimer != 0 && millis() - specModeTimer > 5000)
    {
      specModeTimer = 0;
      specModeEnabled = false;
      specModeClickTimer = 0;
    }
  }

  //спец режим
  if (specModeEnabled)
  {
    if (blickTimer > 300) //индикация спец режима
    {
      if (blickFlag)
      {
        Serial.println("flash on");
        analogWrite(5, chargeLvlR);
        analogWrite(3, chargeLvlG);
      }
      else
      {
        Serial.println("flash off");
        digitalWrite(5, HIGH);
        digitalWrite(3, HIGH);
      }
      blickFlag = !blickFlag;
      blickTimer = 0;
    }
    blickTimer++;
    digitalWrite(11, LOW);
    if (specMotor) motor.writeMicroseconds(2200);
    else motor.writeMicroseconds(motorStopVal);
    return;
  } else
  {
    //индикация включения + инициализация мотора
    if (!enabled) {
      motor.writeMicroseconds(motorStopVal);
      digitalWrite(5, HIGH);
      digitalWrite(3, HIGH);
      digitalWrite(11, HIGH); //выключение записи
    } else {
      analogWrite(5, chargeLvlR);
      analogWrite(3, chargeLvlG);
      digitalWrite(11, LOW); //включение записи
    }
  }

  //индикация датчика холла
  if (sensHall1) {
    if (h1Micros == 0)
      h1Micros = micros();
    digitalWrite(10, LOW);
  } else {
    if (h1Micros > 0)
    {
      lastTriggerHallMicros = micros() - h1Micros;
      h1Micros = 0;
    }
    digitalWrite(10, HIGH);
    if (sensHall2) {
      digitalWrite(10, LOW);
    }
    else {
      if (h1Micros > 0)
      {
        lastTriggerHallMicros = micros() - h1Micros;
        h1Micros = 0;
      }
      digitalWrite(10, HIGH);
    }
  }

  //счетчик срабатываний датчика холла
  static uint32_t tmr;
  if (enabled && millis() - tmr >= 5) {
    tmr = millis();
    if (sensHall1) {
      h1DurTime += 5;
      lastTriggerHall = millis();
      startHall1 = true;
    }
    else if (sensHall2) {
      h2DurTime += 5;
      lastTriggerHall = millis();
      startHall2 = true;
    }
    else
    {
      if (h1DurTime > 0)
        numTriggeredH = 1;
      if (h2DurTime > 0)
        numTriggeredH = 2;
      h1DurTime = 0;
      h2DurTime = 0;
    }
  }

  if (lastTriggerHallMicros > 0) {
    values[index] = lastTriggerHallMicros;
    index++;
    if (index > numReading - 1)
      index = 0;
    uint64_t total = 0;

    bool valuesFill = true;
    for (int i = 0; i < numReading; i++)
    {
      if (values[i] == 0)
        valuesFill = false;
      total += values[i];
    }
    average = total / numReading;
    powerMotor = p1;
    if (valuesFill)
    {
      // ver 1
      /*if (average > d1)
        {
        powerMotor = (d1*p2) / average;
        }
        else if (average > d2 && average <= d1)
        {
        powerMotor = p2;
        }
        else if (average > d3 && average <= d2)
        {
        powerMotor = 2800 - 0.04 * average;
        }
        else if (average <= d3)
        {
        powerMotor = p3;
        }*/

      //ver 2
      /*powerMotor = koeff / average  * p3;
        Serial.print("powerMotor calc1: ");
        Serial.println(powerMotor);
        powerMotor = map(powerMotor, 2000, 900, 2200, 1550);
        Serial.print("average: ");
        Serial.println(average);
        Serial.print("powerMotor calc2: ");
        Serial.println(powerMotor);*/

      //ver 3
      if ((koeff / average  * p3) < 2000) {
        powerMotor = 1900;
      }
      if ((koeff / average  * p3) > 2000) {
        powerMotor = 2200;
      }
    }
    if (powerMotor < p1)
      powerMotor = p1;

    if (powerMotor >= p3)
      powerMotor = p3;

    Serial.print("powerMotor: ");
    Serial.println(powerMotor);
    Serial.print("koeff: ");
    Serial.println(koeff / average  * p3);

    lastTriggerHallMicros = 0;
  }


  //Serial.print(sensHall1);
  //Serial.print("   ");
  //Serial.print(h2DurTime);
  //Serial.print("   ");
  //Serial.println(sensHall2);
  //Serial.print("   ");
  //Serial.println(h1DurTime);

  // включаем мотор после срабатывания 2 датчиков(оборот)
  if (startHall1 && startHall2)
  {
    motor.writeMicroseconds(powerMotor);
    motorEnabled = true;
    startHall1 = false;
    startHall2 = false;
  }

  static uint32_t tmr3;
  // выключаем мотор после бездействия
  int dur = 1100;
  int f1 = millis() - lastTriggerHall >= dur + dur / 5;
  bool f2 = h2DurTime >= dur;
  bool f3 = h1DurTime >= dur;
  if (enabled && motorEnabled && ( f1 || f2 || f3 )) {
    Serial.print("Stop motor ");
    Serial.print(f1);
    Serial.print(" ");
    Serial.print(f2);
    Serial.print(" ");
    Serial.println(f3);
    motor.writeMicroseconds(motorStopVal);
    motorEnabled = false;
    h2DurTime = 0;
    h1DurTime = 0;
    startHall1 = false;
    startHall2 = false;
    for (int i = 0; i < numReading; i++)
    {
      values[i] = 0;
    }
  }
}

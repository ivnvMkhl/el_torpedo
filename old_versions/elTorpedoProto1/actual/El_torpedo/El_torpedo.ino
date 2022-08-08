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
uint32_t values[numReading];
int index = 0;
uint32_t average = 0;
uint32_t d1 = 50000;
uint32_t d2 = 25000;
uint32_t d3 = 15000;
int p1 = 1550;
int p2 = 1800;
int p3 = 2200;
int powerMotor;
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
  static bool enabled;
  static bool h1_signal;
  static bool h2_signal;
  static int h2_dur_time;
  static int h1_dur_time;
  static bool start_h1;
  static bool start_h2;
  static uint32_t last_trigger_hall;
  static int mstop = 950;
  static int mgo = 1550;
  static int cycle = 5000;
  static int ch_lvl_R;
  static int ch_lvl_G;
  static int num_triggered_h = -1;
  int charge_level = analogRead(A0);
  static int charge_level_1;
  static bool motor_enabled = false;
  static uint64_t h1_micros = 0;
  static uint64_t last_trigger_hall_micros;
  static uint32_t specModeTimer = 0;
  static int specModeClickTimer = 0;
  static bool specModeEnabled = false;
  static int blickTimer = 0;
  static bool blickFlag = false;

  // ремап цветовой индикации
  charge_level_1 = map(charge_level, 0, 1023, 630, 840);
  ch_lvl_R = map(charge_level, 750, 630, 253, 0);
  ch_lvl_R = constrain(ch_lvl_R, 0, 253);
  ch_lvl_G = map(charge_level, 700, 840, 253, 0);
  ch_lvl_G = constrain(ch_lvl_G, 0, 253);

  //каждый луп смотрю состояние датчиков холла
  h1_signal = !digitalRead(6);
  h2_signal = !digitalRead(8);
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
        analogWrite(5, ch_lvl_R);
        analogWrite(3, ch_lvl_G);
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
    else motor.writeMicroseconds(mstop);
    return;
  } else
  {
    //индикация включения + инициализация мотора
    if (!enabled) {
      motor.writeMicroseconds(mstop);
      digitalWrite(5, HIGH);
      digitalWrite(3, HIGH);
      digitalWrite(11, HIGH); //выключение записи
    } else {
      analogWrite(5, ch_lvl_R);
      analogWrite(3, ch_lvl_G);
      digitalWrite(11, LOW); //включение записи
    }
  }

  //индикация датчика холла
  if (h1_signal) {
    if (h1_micros == 0)
      h1_micros = micros();
    digitalWrite(10, LOW);
  } else {
    if (h1_micros > 0)
    {
      last_trigger_hall_micros = micros() - h1_micros;
      h1_micros = 0;
    }
    digitalWrite(10, HIGH);
    if (h2_signal) {
      digitalWrite(10, LOW);
    }
    else {
      if (h1_micros > 0)
      {
        last_trigger_hall_micros = micros() - h1_micros;
        h1_micros = 0;
      }
      digitalWrite(10, HIGH);
    }
  }

  //счетчик срабатываний датчика холла
  static uint32_t tmr;
  if (enabled && millis() - tmr >= 5) {
    tmr = millis();
    if (h1_signal) {
      /*if (num_triggered_h == 1) {
        Serial.println("Stop motor 'num_triggered_h == 1'");
        motor.writeMicroseconds(mstop);
        h1_dur_time = 0;
        }
        else {*/
      h1_dur_time += 5;
      last_trigger_hall = millis();
      start_h1 = true;
      //}
    }
    else if (h2_signal) {
      /*if (num_triggered_h == 2) {
        Serial.println("Stop motor 'num_triggered_h == 2'");
        motor.writeMicroseconds(mstop);
        h2_dur_time = 0;
        }
        else {  */
      h2_dur_time += 5;
      last_trigger_hall = millis();
      start_h2 = true;
      //}
    }
    else
    {
      if (h1_dur_time > 0)
        num_triggered_h = 1;
      if (h2_dur_time > 0)
        num_triggered_h = 2;
      h1_dur_time = 0;
      h2_dur_time = 0;
    }
  }

  if (last_trigger_hall_micros > 0) {
    values[index] = last_trigger_hall_micros;
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

    last_trigger_hall_micros = 0;
  }


  //Serial.print(h1_signal);
  //Serial.print("   ");
  //Serial.print(h2_dur_time);
  //Serial.print("   ");
  //Serial.println(h2_signal);
  //Serial.print("   ");
  //Serial.println(h1_dur_time);

  // включаем мотор после срабатывания 2 датчиков(оборот)
  if (start_h1 && start_h2)
  {
    motor.writeMicroseconds(powerMotor);
    motor_enabled = true;
    start_h1 = false;
    start_h2 = false;
  }

  static uint32_t tmr3;
  // выключаем мотор после бездействия
  int dur = 1100;
  int f1 = millis() - last_trigger_hall >= dur + dur / 5;
  bool f2 = h2_dur_time >= dur;
  bool f3 = h1_dur_time >= dur;
  if (enabled && motor_enabled && ( f1 || f2 || f3 )) {
    Serial.print("Stop motor ");
    Serial.print(f1);
    Serial.print(" ");
    Serial.print(f2);
    Serial.print(" ");
    Serial.println(f3);
    motor.writeMicroseconds(mstop);
    motor_enabled = false;
    h2_dur_time = 0;
    h1_dur_time = 0;
    start_h1 = false;
    start_h2 = false;
    for (int i = 0; i < numReading; i++)
    {
      values[i] = 0;
    }
  }
}

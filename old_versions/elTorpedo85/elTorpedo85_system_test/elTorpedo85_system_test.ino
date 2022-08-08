/*
  pinOUT
  2 - КНОПКА
  3 - СД ЗЕЛЕНЫЙ
  4 - Сигнал зарядки +
  5 - СД КРАСНЫЙ
  6 - ДАТЧИК ХОЛЛА 1
  7 - МОТОР
  8 - ДАТЧИК ХОЛЛА 2
  9 - СД СИНИЙ
  10 -
  11 - //ВКЛ ЗАПИСИ ВЫХОД
  12 - //ВКЛ МОТОРА ВХОД
  13 - //ВКЛ МОТОРА ВХОД 2
  А0 - уровень напряжения

  1 - +
  2 - -
  3,4 - sign
*/

#include <Servo.h>

Servo motor;

//  var init
bool hallSens = false, speedSwitch = false, speedSwitch2 = false, speedArr[] = {true, false};
int ledR = 5, ledG = 3, ledB = 10, hallSensPin = 6, motorPin = 7, motorStop = 800, motorSpeedArr[] = {1500, 2200}, i = 0, n = 0,
    chargeLvl = 0, chargeLvlMap = 0, chargeLvlR = 0, chargeLvlG = 0;

bool enabled = true, motorEnabled = false;


void setup() {


  motor.attach(motorPin); 
  Serial.begin(9600);

  pinMode(ledR, OUTPUT); //RED
  pinMode(ledG, OUTPUT); //Green
  pinMode(ledB, OUTPUT); //Blue
  pinMode(hallSensPin, INPUT_PULLUP); //hallSens

  //  LED off initialisation (LOW - off, HIGH - on)
  digitalWrite(ledR, HIGH);
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);

  //  Motor ON initialisation 3 sec waiting
  motor.writeMicroseconds(motorStop);
  delay(3000);
}

void loop() {

  //update sensor status every loop
  hallSens = !digitalRead(6);
  chargeLvl = analogRead(A0);

  //  Serial.println(hallSens);


  //indication sensor blue led
  if (hallSens) {
    digitalWrite(ledB, HIGH);
  } else {
    digitalWrite(ledB, LOW);
  }

  //индикация включения + инициализация мотора
  if (!enabled) {
    motor.writeMicroseconds(motorStop);
    digitalWrite(ledR, LOW);
    digitalWrite(ledG, LOW);
    i = 0;
  } else {
    i = 0;
    chargeLvlMap = map(chargeLvl, 0, 1023, 630, 840);
    chargeLvlG = map(chargeLvlMap, 750, 630, 253, 0);
    chargeLvlG = constrain(chargeLvlG, 0, 253);
    chargeLvlR = map(chargeLvlMap, 700, 840, 253, 0);
    chargeLvlR = constrain(chargeLvlR, 0, 253);
    analogWrite(ledR, chargeLvlR);
    analogWrite(ledG, chargeLvlG);

    //Charge indicator calibration, low(chargeLvl) = 20v(790), high(chargeLvl) = ,
    //    Serial.print(chargeLvl);
    //    Serial.print("   ");
    //    Serial.print(chargeLvlMap);
    //    Serial.print("   ");
    //    Serial.print(chargeLvlR);
    //    Serial.print("   ");
    //    Serial.println(chargeLvlG);

    if (hallSens && speedArr[0])  {
      motor.writeMicroseconds(motorSpeedArr[0]);
      speedSwitch2 = true;
      delay(20);
      return;
    }
    else if (hallSens && speedArr[1])  {
      motor.writeMicroseconds(motorSpeedArr[1]);
      speedSwitch2 = true;
      delay(20);
      return;
    } else {
      if (speedSwitch2) {
        delay(50);
        while (i < 100) {
          hallSens = !digitalRead(6);
          if (hallSens && !speedSwitch) {
            speedSwitch = true;
            speedSwitch2 = false;
            speedArr[0] = !speedArr[0];
            speedArr[1] = !speedArr[1];
            return;
          }
          if (!hallSens && speedSwitch) {
            speedSwitch = false;
          }
          i++;
          delay(10);
        }
      }
      motor.writeMicroseconds(motorStop);
      speedArr[0] = true;
      speedArr[1] = false;
      speedSwitch2 = false;
    }
  }
}

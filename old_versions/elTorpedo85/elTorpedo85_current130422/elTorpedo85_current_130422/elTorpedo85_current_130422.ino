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
bool hallSens = false, speedSwitch = false, speedSwitch2 = false, speedArr[] = {true, false, false};
int ledR = 5, ledG = 3, ledB = 10, hallSensPin = 6, motorPin = 7, motorStop = 800, motorSpeedArr[] = {1360, 1530, 1800}, i = 0, n = 0,
    chargeLvl = 0, chargeLvlMap = 0, chargeLvlR = 0, chargeLvlG = 0;

bool enabled = true, motorEnabled = false;


void setup() {

  motor.attach(motorPin);                                     // Motor initial

  Serial.begin(9600);

  pinMode(ledR, OUTPUT);                                      // Red LED pin init
  pinMode(ledG, OUTPUT);                                      // Green ~
  pinMode(ledB, OUTPUT);                                      // Blue ~

  pinMode(hallSensPin, INPUT_PULLUP);                         // hallSens pin init

  digitalWrite(ledR, HIGH);                                   //  LED off initialisation (LOW - off, HIGH - on)
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);

  motor.writeMicroseconds(motorStop);                         // Motor ON initialisation 3 sec waiting
  delay(3000);                                                // Start latency 3000ms

}

void loop() {

  hallSens = !digitalRead(hallSensPin);                       // update hallSens status every loop
  chargeLvl = analogRead(A0);                                 // update voltageSens status every loop

  if (!enabled) {                                             // power switch off
    i = 0;
    motor.writeMicroseconds(motorStop);
    digitalWrite(ledR, LOW);
    digitalWrite(ledG, LOW);
  } else {                                                    // power switch on
    i = 0;

    // 950 - 22v ; 1023 - 24,8v ;
    chargeLvlMap = map(chargeLvl, 950, 1023, 0, 1023);
    chargeLvlMap = constrain(chargeLvlMap, 0, 1023);

    chargeLvlG = map(chargeLvlMap, 1023, 300, 253, 0);
    chargeLvlG = constrain(chargeLvlG, 0, 253);

    chargeLvlR = map(chargeLvlMap, 800, 0, 0, 253);
    chargeLvlR = constrain(chargeLvlR, 0, 253);

    analogWrite(ledR, chargeLvlR);
    analogWrite(ledG, chargeLvlG);

    // Charge indicator calibration, low(chargeLvl) = 20v(790), high(chargeLvl) = ,
    //        Serial.print(chargeLvl);
    //        Serial.print("   ");
    //        Serial.print(chargeLvlMap);
    //        Serial.print("   ");
    //        Serial.print(chargeLvlR);
    //        Serial.print("   ");
    //        Serial.println(chargeLvlG);

    if (hallSens && speedArr[0])  {                           // start on 1 speed
      motor.writeMicroseconds(motorSpeedArr[0]);
      speedSwitch2 = true;
      delay(20);                                              // for smooth speed swith
      return;
    }

    else if (hallSens && speedArr[1])  {                      //start on 2 speed
      motor.writeMicroseconds(motorSpeedArr[1]);
      speedSwitch2 = true;
      delay(20);                                              // for smooth speed swith
      return;

    }

    else if (hallSens && speedArr[2])  {                      //start on 2 speed
      motor.writeMicroseconds(motorSpeedArr[2]);
      speedSwitch2 = true;
      delay(20);                                              // for smooth speed swith
      return;

    }

    else {

      if (speedSwitch2) {                                     // start latency
        delay(50);                                            // latency for smooth speed swith
        while (i < 50) {                                      // cycles for check speed switch event // 100 - 1000ms // 50 - 500ms
          hallSens = !digitalRead(hallSensPin);               // refresh hallSens signal
          if (hallSens && !speedSwitch && speedArr[0]) {
            speedSwitch = true;
            speedSwitch2 = false;
            speedArr[0] = false;
            speedArr[1] = true;
            speedArr[2] = false;
            return;
          }
          if (hallSens && !speedSwitch && speedArr[1]) {
            speedSwitch = true;
            speedSwitch2 = false;
            speedArr[0] = false;
            speedArr[1] = false;
            speedArr[2] = true;
            return;
          }
          if (hallSens && !speedSwitch && speedArr[2]) {
            speedSwitch = true;
            speedSwitch2 = false;
            speedArr[0] = true;
            speedArr[1] = false;
            speedArr[2] = false;
            return;
          }
          if (!hallSens && speedSwitch) {
            speedSwitch = false;
          }
          i++;
          delay(10);                                          // latency for check speed switch
        }
      }
      // motor
      motor.writeMicroseconds(motorStop);
      speedArr[0] = true;
      speedArr[1] = false;
      speedArr[2] = false;
      speedSwitch2 = false;
    }
  }
}

/*
  pinOUT
  2 - Charging relay
  3 - LED Green
  4 - 
  5 - LED Red
  6 - Hall Sensor
  7 - motor ESC
  8 - 
  9 - LED Blue
  10 -
  11 - 
  12 - 
  13 - 
  А0 - Charging lvl
*/

#include <Arduino.h>
#include <Servo.h>

Servo motor;

//  var init
bool hallSens = false, 
     speedSwitch = false, 
     speedSwitch2 = false, 
     speedArr[] = {true, false};

int ledR = 3, 
    ledG = 5, 
    ledB = 9, 
    chargingPin = 4,
    hallSensPin = 6, 
    motorPin = 7, 
    motorStop = 800, 
    motorSpeedArr[] = {1460, 1700}, 
    i = 0, 
    n = 0,
    chargeLvl = 0, 
    chargeLvlMap = 0, 
    chargeLvlR = 0, 
    chargeLvlG = 0;

bool enabled = true, 
     motorEnabled = false,
     charging = false;


void setup() {

  motor.attach(motorPin);                                     // Motor initial

  Serial.begin(9600);

  pinMode(ledR, OUTPUT);                                      // Red LED pin init
  pinMode(ledG, OUTPUT);                                      // Green ~
  pinMode(ledB, OUTPUT);                                      // Blue ~

  pinMode(hallSensPin, INPUT_PULLUP);                         // hallSens pin init
  pinMode(chargingPin, INPUT_PULLUP);                         // charging pin init

  digitalWrite(ledR, HIGH);                                   //  LED off initialisation (LOW - off, HIGH - on)
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, LOW);

  motor.writeMicroseconds(motorStop);                         // Motor ON initialisation 3 sec waiting

  // Start latency 3500ms
  delay(500);
  digitalWrite(ledB, HIGH);
  delay(500);
  digitalWrite(ledB, LOW);
  delay(500);
  digitalWrite(ledB, HIGH);
  delay(500);
  digitalWrite(ledB, LOW);
  delay(500);
  digitalWrite(ledB, HIGH);
  delay(500);
  digitalWrite(ledB, LOW);
  delay(500);
  digitalWrite(ledB, HIGH);
}

void loop() {

  hallSens = !digitalRead(hallSensPin);                       // update hallSens status every loop
  chargeLvl = analogRead(A0);                                 // update voltageSens status every loop
  

  if (!digitalRead(chargingPin)) {                                             // charging mode
    
    motor.writeMicroseconds(motorStop);
    

    if (chargeLvl < 985) {
    chargeLvlMap = map(chargeLvl, 950, 980, 0, 1023);
    chargeLvlMap = constrain(chargeLvlMap, 0, 1023);

    chargeLvlG = map(chargeLvlMap, 1023, 300, 253, 0);
    chargeLvlG = constrain(chargeLvlG, 0, 253);

    chargeLvlR = map(chargeLvlMap, 800, 0, 0, 253);
    chargeLvlR = constrain(chargeLvlR, 0, 253);

    analogWrite(ledR, chargeLvlR);
    analogWrite(ledG, chargeLvlG);
    
    delay(500);

    digitalWrite(ledR, HIGH);
    digitalWrite(ledG, HIGH);
    
    delay(500);
    } else {

    chargeLvlMap = map(chargeLvl, 950, 1023, 0, 1023);
    chargeLvlMap = constrain(chargeLvlMap, 0, 1023);

    chargeLvlG = map(chargeLvlMap, 1023, 300, 253, 0);
    chargeLvlG = constrain(chargeLvlG, 0, 253);

    chargeLvlR = map(chargeLvlMap, 800, 0, 0, 253);
    chargeLvlR = constrain(chargeLvlR, 0, 253);

    analogWrite(ledR, chargeLvlR);
    analogWrite(ledG, chargeLvlG);
    }

    
  } else {                                                    // main program
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
            return;
          }
          if (hallSens && !speedSwitch && speedArr[1]) {
            speedSwitch = true;
            speedSwitch2 = false;
            speedArr[0] = false;
            speedArr[1] = false;
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
      speedSwitch2 = false;
    }
  }
}

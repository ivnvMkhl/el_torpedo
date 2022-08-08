#include <EEPROM.h>

int chargeLvlEEPROM = 0;
int readEEPROMdata = 0;
int adrCounter = 0;
int i = 0;

void setup() {

  Serial.begin(9200);

  //READ
  Serial.println("===========================================");
  for (i=0; i<10; i++) {
    EEPROM.get(adrCounter, readEEPROMdata);
    Serial.print("Get on EEPROM to address: ");
    Serial.print(adrCounter / 2);
    Serial.print(" Data: ");
    Serial.println(readEEPROMdata);
    adrCounter = adrCounter + 2;
  }
  Serial.println("===========================================");
  Serial.println("");
  adrCounter = 0;
}

void loop() {
  
  //WRITE
  static uint32_t tmr;
  if (millis() - tmr >= 3000) {
    tmr = millis();
    if (adrCounter < 20) {
      chargeLvlEEPROM =  analogRead(A0); 
      EEPROM.put(adrCounter, chargeLvlEEPROM);
      Serial.print("Put on EEPROM to address: ");
      Serial.print(adrCounter / 2);
      Serial.print(" Data: ");
      Serial.println(chargeLvlEEPROM);

      adrCounter = adrCounter + 2;
    }
  }

}

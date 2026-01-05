#include <EEPROM.h>

int inputPins[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };
int votes[8];

int confirmLed = 12, confirmBtn = 11;  //10

String names[8];

int eepromAdd = 0;

void setup() {
  Serial.begin(9600);

  //setting pins 2-9 as input for the buttons
  for (int i = 0; i < 8; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  //setting pin 11 as confirm and 12 as led
  pinMode(confirmBtn, INPUT_PULLUP);
  pinMode(confirmLed, OUTPUT);
}

int currInput[8];

void loop() {

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'S') {
      setCandidateNames();
    }
  }

  // gettin input from each button
  for (int i = 0; i < 8; i++) {
    currInput[i] = digitalRead(inputPins[i]);
  }

  for (int i : currInput) {

    if (i == LOW) {

      //check for if candidates name has been set
      boolean flag = false;
      for (String s : names) {
        if (s == NULL) flag = true;
      }
      if (flag == true) {
        Serial.println("Candidates name not set!");
        continue;
      }


      digitalWrite(confirmLed, HIGH);

      // waiting confirmation
      while (digitalRead(confirmBtn) == HIGH) {
        Serial.println("waiting for confirmation");
      }

      votes[i] += 1;
      storeToEEPROM();

      if (digitalRead(confirmBtn) == LOW) {
        digitalWrite(confirmLed, LOW);
      }

      //debug
      // Serial.print(i);
      // Serial.print(":");
      // Serial.println(votes[i]);

      delay(300);
      break;
    }
  }
}

void storeToEEPROM() {

  for (int i = 0; i < 8; i++) {
    EEPROM.put(eepromAdd, votes[i]);
    eepromAdd += sizeof(int);
  }

  eepromAdd = 0;
}



void clearEEPROM() {

  for (int i = 0; i < 8; i++) {
    EEPROM.put(eepromAdd, 0);
    eepromAdd += sizeof(int);
  }

  eepromAdd = 0;
}

void setCandidateNames() {

  //getting name of each candidate, nil for no candidate
  for (int i = 0; i < 8; i++) {
    Serial.print("Candidate ");
    Serial.print(i + 1);
    Serial.print(": ");

    while (Serial.available() == 0) {
    }

    String s = Serial.readStringUntil('\n');
    if (s.equals("")) {
      names[i] = "nil";
    } else {
      names[i] = Serial.readStringUntil('\n');
    }
    Serial.println(names[i]);
  }

  Serial.println("Inputs set");
}
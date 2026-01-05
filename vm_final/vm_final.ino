#include <EEPROM.h>

int inputPins[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };
int votes[8];
int currInput[8];

int confirmLed = 12, confirmBtn = 11;  //11

String names[8];

int eepromAdd = 0;
int setAdd = 100;

void setup() {
  Serial.begin(9600);

  //setting pins 2-9 as input for the buttons
  for (int i = 0; i < 8; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  //setting pin 10 and 11 as output for the buzzers and 12 as input for confirmation
  pinMode(confirmBtn, INPUT_PULLUP);
  pinMode(confirmLed, OUTPUT);

  //write to identifying address to eeprom as not set
  saveStateToEEPROM();
}

void loop() {
  //getting user commands
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'S') {
      setCandidateNames();
    } else if (c == 'A') {
      showAll();
    } else if (c == 'W') {
      calculateWinner();
    } else if (c == 'C') {
      clearData();
    }
  }

  //checking if candidates name has been set
  if (hasZeroInEEPROM()) {
    //debugging
    digitalWrite(confirmLed, HIGH);
    delay(100);
    digitalWrite(confirmLed, LOW);
    delay(100);
    digitalWrite(confirmLed, HIGH);
    delay(100);
    digitalWrite(confirmLed, LOW);

    Serial.println("Candidates name not set!");
  } else {

    // gettin input from each button
    for (int i = 0; i < 8; i++) {
      currInput[i] = digitalRead(inputPins[i]);
    }

    //loops over the current states of the button
    for (int i : currInput) {

      if (i == LOW) {
        int t;
        EEPROM.get(setAdd + (i * sizeof(int)), t);
        if (t == 2) {
          //temporary blink set up for debugging
          digitalWrite(confirmLed, HIGH);
          delay(200);
          digitalWrite(confirmLed, LOW);
          continue;

        } else {

          digitalWrite(confirmLed, HIGH);

          // waiting confirmation
          while (digitalRead(confirmBtn) == HIGH) {
            char c = Serial.read();
            if (c == 'O') {
              break;
            }
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
  }
}

//additional methods
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

void getFromEEPROM() {

  for (int i = 0; i < 8; i++) {
    votes[i] = EEPROM.read(eepromAdd);
    eepromAdd += sizeof(int);
  }

  eepromAdd = 0;
}

void saveStateToEEPROM() {
  for (int i = setAdd; i < setAdd + (sizeof(int) * 8); i += (sizeof(int))) {
    if (names[i] == NULL) {
      EEPROM.write(setAdd, 0);
    } else if (names[i].equals("nil")) {
      EEPROM.write(setAdd, 2);
    } else {
      EEPROM.write(setAdd, 1);
    }
  }
  setAdd = 0;
}

bool hasZeroInEEPROM() {
  for (int i = setAdd; i < setAdd + (sizeof(int) * 8); i += (sizeof(int))) {
    if (EEPROM.read(i) == 0) {
      return true;
    }
  }
  setAdd = 0;
  return false;
}

void setCandidateNames() {
  Serial.readStringUntil('\n'); // Because the command will send a '\n'

  //getting name of each candidate, nil for no candidate
  for (int i = 0; i < 8; i++) {
    Serial.print("Candidate ");
    Serial.print(i + 1);
    Serial.print(": ");

    while (Serial.available() == 0) {}

    String s = Serial.readStringUntil('\n');
    if (s.equals("")) {
      names[i] = "nil";
    } else {
      names[i] = s;
    }
    Serial.println(names[i]);
  }

  saveStateToEEPROM();
  Serial.println("Inputs set");
}

//displays each candidate and their votes
void showAll() {
  getFromEEPROM();
  for (int i = 0; i < 8; i++) {
    String s = "Candidate ";
    s += i + 1;
    s += ":";
    s += votes[i];

    Serial.println(s);
  }
}

//sorts the votes and shows the winners
void calculateWinner() {
  getFromEEPROM();

  int winner = 0;
  for (int i = 0; i < 8; i++) {
    if (votes[i] > votes[winner]) {
      winner = i;
    }
  }
  ///handle ties

  Serial.print("Winner : ");
  Serial.print("Candidate ");
  Serial.print(winner + 1);
  Serial.print(" - ");
  Serial.println(votes[winner]);
}

//Clears all data to use the machine for the next election
void clearData() {
  Serial.print("Press 'Y' to clear all data or 'N' to cancel");

  delay(1000);
  while (Serial.available() == 0) {}

  char c = Serial.read();
  if (c == 'Y') {

    for (int i = 0; i < 8; i++) {
      EEPROM.put(eepromAdd, 0);
      eepromAdd += sizeof(int);
      EEPROM.put(setAdd, 0);
      setAdd += sizeof(int);
    }

    eepromAdd = 0;
    setAdd = 0;

    Serial.println("Cleared all data");
  } else if (c == 'N') {
    Serial.println("Did not clear data");
  }
}

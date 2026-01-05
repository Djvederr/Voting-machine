#include <EEPROM.h>

//memory address of where the data is stored
int eepromAdd = 0;
int setFlagAdd = 100;

int votes[8];

//___to send commands from the laptop after the election is over___
//COMMANDS[all capital characters]
//            S - show all votes
//            W - Winner result
//            C - clear eeprom
void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'S') {
      showAll();
    } else if (c == 'W') {
      calculateWinner();
    } else if (c == 'C') {
      clearData();
    }
  }
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

  Serial.print("Winner : ");
  Serial.print("Candidate ");
  Serial.print(winner + 1);
  Serial.print(" - ");
  Serial.println(votes[winner]);
}

//Clears all data to use the machine for the next election
void clearData() {
  Serial.println("Press 'Y' to clear all data or 'N' to cancel");

  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == 'Y') {

        for (int i = 0; i < 8; i++) {
          EEPROM.put(eepromAdd, 0);

          eepromAdd += sizeof(int);
        }
        EEPROM.put(setFlagAdd, 0);

        eepromAdd = 0;

        Serial.println("Cleared all data");
      } else if (c == 'N') {
        Serial.println("Did not clear data");
      }

      break;
    }
  }
}

void getFromEEPROM() {

  for (int i = 0; i < 8; i++) {
    votes[i] = EEPROM.read(eepromAdd);
    eepromAdd += sizeof(int);
  }

  eepromAdd = 0;
}
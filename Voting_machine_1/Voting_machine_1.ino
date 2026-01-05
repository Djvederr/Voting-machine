#include <EEPROM.h>

int inputPins[8] = {2,3,4,5,6,7,8,9};
int votes[8];

int confirmLed = 12, confirmBtn = 11;

int latestIdx;  //last voted index

String names[8];

int eepromAdd = 0;

void setup() {
  Serial.begin(9600);
  //setting pins 2-9 as input for the buttons
  for (int i = 0; i < 8; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  //setting pin 10 and 11 as output for the buzzers and 12 as input for confirmation
  pinMode(confirmBtn, INPUT_PULLUP);
  pinMode(confirmLed, OUTPUT);

}

int currInput[8];

void loop() {

  //----- to send commands from the laptop after the election is over -----
  //COMMANDS -  R(82) - to revert lastest vote    //only use once.
  //            A(65) - to show all results
  //            V(86) - //ignore
  //            W(87) - Winner result
  //            C(67) - clear e8...........................................................8833333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333338/eprom
  //            S(83)   - to set names
  if (Serial.available() > 0) {
    //gets the input
    int br = Serial.read();
    //Serial.println(br);

    if(br == 67){ // 'C'
        clearEEPROM();
    }

    if(br == 86){
      //doesnt work yet
      int ar[] = {0,1,3};
      calculateResult(ar, sizeof(ar));
    }
    if(br == 83) {
      setNames();
    }

    if(br == 87){
        calculateResultAll();
    }

    if (br == 82) {
        votes[latestIdx] -= 1;
        Serial.println("Reverted Result");

        calculateGeneralResult();
    }

    if (br == 65) {
      calculateGeneralResult();
    }
  }
  //----------------------------------------------------------------------

  // gettin input from each button
  for (int i = 0; i < 8; i++) {
    currInput[i] = digitalRead(inputPins[i]);
  }

  for (int i = 0; i < 8; i++) {

    if (currInput[i] == LOW) {
      digitalWrite(confirmLed, HIGH);

      // waiting confirmation
      while(digitalRead(confirmBtn) == HIGH){
        Serial.println("waiting for confirmation");
      }

      votes[i] += 1;
      storeToEEPROM();

      latestIdx = i;

      if(digitalRead(confirmBtn) == LOW){
        digitalWrite(confirmLed, LOW);
      }   
   
      //debug   
      // Serial.print(i);   
      // Serial.print(":"   );
      // Serial.println(v   otes[i]);
   
      delay(300);   
      break;   
      }   
    }   
  }   
   
void setNames() {
  Serial.readStringUntil('\n');
  for (int i = 0; i < 8; i++) {

    Serial.print("Candidate ");
    Serial.print(i+1);
    Serial.print(": ");

    while (Serial.available() == 0) {
    }
    names[i] = Serial.readStringUntil('\n');
    Serial.println(names[i]);
  }
  Serial.println("Inputs set");
}
void calculateResultAll()   {
  getFromEEPROM();   
  int winner = 0;   
  for(int i=0;i<8;i++){
    if(votes[i] > votes[winner]){
      winner = i;
    }
  }

  Serial.print("Winner : ");
  Serial.print(names[winner]);
  Serial.print(" - ");
  Serial.println(votes[winner]);
}

//not finshed, not needed, dont delete
void calculateResult(int ar[], int n){
  int temp[n];
  for(int i=0;i<n;i++){
    temp[i] = votes[ar[i]];
  }

  int winner = 0;
  for(int i=0;i<n;i++){
    if(temp[i] > temp[winner]){
      winner = i;
    }
  }

  Serial.print("Winner : ");
  Serial.print(names[winner]+"   ");
  Serial.print(" - ");
  Serial.println(temp[winner]);
}

void calculateGeneralResult() {

  getFromEEPROM();

  for (int i = 0; i < 8; i++) {
    String s = names[i];
    s += " (Candidate ";
    s += i+1;
    s += "):  ";
    s += votes[i];

    Serial.println(s);
  }

}

void storeToEEPROM(){

  for(int i=0;i<8;i++){
    EEPROM.put(eepromAdd, votes[i]);
    eepromAdd += sizeof(int);
  }

  eepromAdd = 0;
}

void getFromEEPROM(){

  for(int i=0;i<8;i++){
    votes[i] = EEPROM.read(eepromAdd);
    eepromAdd += sizeof(int);
  }
  
  eepromAdd = 0;
}

void clearEEPROM(){
  for(int i=0;i<8;i++){
    EEPROM.put(eepromAdd, 0);
    eepromAdd += sizeof(int);
  }

  eepromAdd = 0;
}
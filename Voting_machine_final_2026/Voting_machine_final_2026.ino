/* * SCHOOL VOTING MACHINE - REMADE & COMMENTED
 * * This program allows for:
 * 1. Setting up 8 Candidates via Serial Monitor.
 * 2. Voting with a "Select -> Confirm" two-button safety mechanism.
 * 3. saving votes to memory (EEPROM) so they aren't lost if power fails.
 * 4. Viewing results and determining the winner.
 */

#include <EEPROM.h>

// --- PIN DEFINITIONS ---
// The buttons for Candidates 1 through 8
int inputPins[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };

// The confirmation button and the status LED
int confirmBtn = 11;
int confirmLed = 12;

// --- VARIABLES ---
int votes[8];         // Stores the vote count for each candidate
String names[8];      // Stores the names (e.g., "Alice", "Bob") - used during setup

// --- MEMORY (EEPROM) ADDRESSES ---
// We use these constants to know where to save data in the Arduino's permanent memory.
const int EEPROM_VOTE_START = 0;      // Votes start at address 0
const int EEPROM_FLAG_START = 100;    // "Name Set" flags start at address 100

void setup() {
  Serial.begin(9600); // Start communication with the computer

  // 1. Setup Candidate Buttons
  // Loop through pins 2-9 and set them as inputs
  for (int i = 0; i < 8; i++) {
    pinMode(inputPins[i], INPUT_PULLUP); // INPUT_PULLUP means LOW = Pressed, HIGH = Not Pressed
  }

  // 2. Setup Confirmation Hardware
  pinMode(confirmBtn, INPUT_PULLUP);
  pinMode(confirmLed, OUTPUT);

  // 3. Initial Memory Check
  // We check if candidate names are set up in memory so the loop knows if it's ready to vote.
  // We don't need to explicitly write "not set" every time we turn on, 
  // we just need to read what is already there.
}

void loop() {
  
  // ==========================================
  // PART 1: CHECK FOR COMPUTER COMMANDS
  // ==========================================
  // If the user types a command in the Serial Monitor, handle it here.
  if (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == 'S') {
      setCandidateNames();  // Type 'S' to setup names
    } else if (c == 'A') {
      showAll();            // Type 'A' to see current votes
    } else if (c == 'W') {
      calculateWinner();    // Type 'W' to see who won
    } else if (c == 'C') {
      clearData();          // Type 'C' to reset everything (Caution!)
    }
  }

  // ==========================================
  // PART 2: CHECK IF READY TO VOTE
  // ==========================================
  // If the machine hasn't been configured with names yet, blink the LED to warn the user.
  if (areNamesMissing()) {
    // Blink LED pattern to indicate "Error: Setup Needed"
    digitalWrite(confirmLed, HIGH); delay(100);
    digitalWrite(confirmLed, LOW);  delay(100);
    digitalWrite(confirmLed, HIGH); delay(100);
    digitalWrite(confirmLed, LOW);
    
    Serial.println("System Locked: Please set candidate names (Type 'S').");
    delay(1000); // Wait a bit so we don't spam the Serial Monitor
    return;      // Skip the rest of the loop
  }

  // ==========================================
  // PART 3: READ BUTTONS & PROCESS VOTES
  // ==========================================
  
  // 1. Loop through all 8 candidate buttons to see if anyone is pressing one
  for (int i = 0; i < 8; i++) {
    
    // Read the specific pin for candidate 'i'
    int buttonState = digitalRead(inputPins[i]);

    // If button is LOW, it means it is PRESSED
    if (buttonState == LOW) {
      
      // CHECK IF CANDIDATE IS VALID
      // We look at the "Flag" in memory. 
      // 0 = Not Set, 1 = Valid Candidate, 2 = "Nil" (Empty slot)
      int statusFlag;
      EEPROM.get(EEPROM_FLAG_START + (i * sizeof(int)), statusFlag);

      if (statusFlag == 2) {
        // If this slot is "Nil" (disabled), just blink quickly and ignore.
        digitalWrite(confirmLed, HIGH);
        delay(200);
        digitalWrite(confirmLed, LOW);
        continue; // Skip to next button
      }

      // CANDIDATE IS VALID - START CONFIRMATION SEQUENCE
      
      // 1. Turn on LED to say "I heard you, please confirm."
      digitalWrite(confirmLed, HIGH);

      // 2. Wait indefinitely until the Confirmation Button is pressed
      // (The button reads LOW when pressed, so we loop while it is HIGH)
      bool escape = false;
      while (digitalRead(confirmBtn) == HIGH) {
        // While waiting, check if user sent 'O' to cancel/override (Debugging feature)
        if (Serial.available() > 0 && Serial.read() == 'O') {
          escape = true; 
          break; 
        }
        // Print message occasionally? (Commented out to avoid spam)
        // Serial.println("Waiting for confirmation..."); 
      }

      // If we broke the loop because of 'O', exit without counting vote
      if (escape) {
        digitalWrite(confirmLed, LOW);
        break;
      }

      // 3. VOTE CONFIRMED!
      // Add 1 to the vote count for this candidate
      votes[i] += 1;
      
      // Save the new count to permanent memory immediately
      storeVotesToEEPROM();

      // 4. Turn off LED
      digitalWrite(confirmLed, LOW);

      // 5. Debounce Delay
      // Wait 0.3 seconds to prevent double-counting if the button is held down
      delay(300); 
      
      // Break the loop so we don't accidentally check other buttons immediately
      break; 
    }
  }
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

// --- MEMORY FUNCTIONS ---

// Saves the current 'votes' array into EEPROM
void storeVotesToEEPROM() {
  int address = EEPROM_VOTE_START;
  for (int i = 0; i < 8; i++) {
    EEPROM.put(address, votes[i]); // We use 'put' because it handles Integers automatically
    address += sizeof(int);        // Move address forward by 2 bytes (size of int)
  }
}

// Loads votes from EEPROM into the 'votes' array
void getVotesFromEEPROM() {
  int address = EEPROM_VOTE_START;
  for (int i = 0; i < 8; i++) {
    EEPROM.get(address, votes[i]); // Use 'get' to match 'put'
    address += sizeof(int);
  }
}

// Checks if names have been set (looks for '0' flags in memory)
bool areNamesMissing() {
  for (int i = 0; i < 8; i++) {
    int flag;
    // Calculate address: Start Point + (Candidate Number * Size of Int)
    int address = EEPROM_FLAG_START + (i * sizeof(int));
    EEPROM.get(address, flag);
    
    if (flag == 0) return true; // Found a '0', meaning this slot is not set up
  }
  return false; // All slots are either '1' (Valid) or '2' (Nil)
}

// Wipes all data (Votes = 0, Flags = 0)
void clearData() {
  // --- FIX START: CLEAR THE BUFFER ---
  // This loop throws away any leftover characters (like the Enter key)
  // from previous commands so they don't trigger the check automatically.
  delay(50); // Small wait to ensure all characters have arrived
  while (Serial.available() > 0) {
    Serial.read();
  }
  // --- FIX END ---

  Serial.println("WARNING: Press 'Y' to delete all data. Press 'N' to cancel.");
  
  // Wait until user types something NEW
  while (Serial.available() == 0) {} 

  char c = Serial.read();
  
  // Handle the input
  if (c == 'Y' || c == 'y') { // Added lowercase 'y' check just in case
    // Clear Votes
    int address = EEPROM_VOTE_START;
    for (int i = 0; i < 8; i++) {
      EEPROM.put(address, 0);
      address += sizeof(int);
    }
    
    // Clear Name Flags
    address = EEPROM_FLAG_START;
    for (int i = 0; i < 8; i++) {
      EEPROM.put(address, 0);
      address += sizeof(int);
    }
    
    Serial.println("Memory Wiped.");
  } else {
    Serial.println("Cancelled.");
  }
}

// --- SETUP FUNCTIONS ---

void setCandidateNames() {
  // Clear buffer
  Serial.readStringUntil('\n'); 

  Serial.println("--- ENTER CANDIDATE NAMES ---");
  Serial.println("Type a name and press Enter. Leave empty for 'nil'.");

  int address = EEPROM_FLAG_START;

  for (int i = 0; i < 8; i++) {
    Serial.print("Candidate ");
    Serial.print(i + 1);
    Serial.print(": ");

    // Wait for user input
    while (Serial.available() == 0) {}

    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove whitespace

    // Logic: Save '1' for valid name, '2' for empty (nil)
    int flagVal = 1;
    if (input.equals("")) {
      names[i] = "nil";
      flagVal = 2; // Mark as disabled
    } else {
      names[i] = input;
      flagVal = 1; // Mark as valid
    }
    
    Serial.println(names[i]);

    // Save the status flag to memory
    EEPROM.put(address, flagVal);
    address += sizeof(int);
  }

  Serial.println("Setup Complete.");
}

// --- RESULT FUNCTIONS ---

void showAll() {
  getVotesFromEEPROM(); // Make sure we have the latest data
  Serial.println("--- CURRENT RESULTS ---");
  for (int i = 0; i < 8; i++) {
    Serial.print("Candidate ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(votes[i]);
  }
}

void calculateWinner() {
  getVotesFromEEPROM(); // Refresh data

  int winnerIndex = 0;
  // Simple check: who has the highest number?
  for (int i = 0; i < 8; i++) {
    if (votes[i] > votes[winnerIndex]) {
      winnerIndex = i;
    }
  }

  Serial.print("WINNER: Candidate ");
  Serial.print(winnerIndex + 1);
  Serial.print(" with ");
  Serial.print(votes[winnerIndex]);
  Serial.println(" votes!");
}
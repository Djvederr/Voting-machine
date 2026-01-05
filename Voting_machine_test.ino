/*
  School Voting Machine (New Program)

  This program is based on the pins from your LATEST code:
  - 8 Candidate Buttons: Pins 2-9 (from 'inputPins')
  - 1 Confirmation Button: Pin 11 (from 'confirmBtn')
  - 1 LED: Pin 12 (from 'confirmLed')

  NEW LOGIC:
  1. Pressing a candidate button (2-9) prints its number (1-8) to the Serial Monitor.
  2. Pressing the confirmation button (11) toggles the LED (12) on/off.

  WIRING:
  This code uses INPUT_PULLUP, matching your 'setup()' function.
  - Buttons: Connect one side to the Pin (e.g., 2) and the other side to GND.
*/

// ---===[ Pin Definitions (from your code) ]===---

// The 8 candidate buttons
const int candidatePins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

// The confirmation button
const int confirmPin = 11;

// The LED
const int ledPin = 12;


// ---===[ Global Variables (for state tracking) ]===---

// Stores the last known state of the 8 candidate buttons (for debouncing)
int lastCandidateState[8];

// Stores the last known state of the confirmation button (for debouncing)
int lastConfirmState;

// Stores the current state of the LED (LOW = OFF, HIGH = ON)
int ledState = LOW;


void setup() {
  // Start Serial Monitor
  Serial.begin(9600);
  Serial.println("--- Voting Machine Online ---");
  Serial.println("Logic: Print button number or toggle LED.");

  // Set up the LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState); // Start with LED off

  // Set up the confirmation button pin (matching your code)
  pinMode(confirmPin, INPUT_PULLUP);
  lastConfirmState = HIGH; // Default state for pullup is HIGH (not pressed)

  // Set up all 8 candidate button pins (matching your code)
  for (int i = 0; i < 8; i++) {
    pinMode(candidatePins[i], INPUT_PULLUP);
    lastCandidateState[i] = HIGH; // Default state is HIGH (not pressed)
  }
}


void loop() {
  
  // --- 1. Check all 8 candidate buttons ---
  for (int i = 0; i < 8; i++) {
    // Read the button's current state
    int currentState = digitalRead(candidatePins[i]);

    // Check if the button was just pressed (went from HIGH to LOW)
    // This prevents spamming the Serial Monitor while holding the button
    if (currentState == LOW && lastCandidateState[i] == HIGH) {
      Serial.print("Candidate Button Pressed: ");
      // We add 1 to 'i' because arrays start at 0, but buttons are 1-8
      Serial.println(i + 1);
    }
    
    // Save the current state for the next loop
    lastCandidateState[i] = currentState;
  }

  
  // --- 2. Check the confirmation button ---
  int currentConfirmState = digitalRead(confirmPin);

  // Check if the button was just pressed (went from HIGH to LOW)
  if (currentConfirmState == LOW && lastConfirmState == HIGH) {
    Serial.println("Confirm Button Pressed. Toggling LED.");

    // This is a simple way to toggle the state
    // If ledState is LOW (0), it becomes HIGH (1).
    // If ledState is HIGH (1), it becomes LOW (0).
    ledState = !ledState;

    // Update the actual LED
    digitalWrite(ledPin, ledState);
  }
  
  // Save the current state for the next loop
  lastConfirmState = currentConfirmState;


  // A small delay helps with "button bounce" (reading one press as many)
  delay(20);
}
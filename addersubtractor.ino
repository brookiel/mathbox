#include <Bounce.h>
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <avr/sleep.h>

Adafruit_7segment matrix = Adafruit_7segment(); // Set up display
byte adcsra_save = ADCSRA;

// Specify pins for inputs and outputs
const int addInterrupt = 2;
const int subtractInterrupt = 3;
const int addButtonPin = 21; 
const int subButtonPin = 20; 
const int addLedPin = 25; 
const int subLedPin = 24; 
const int wakeSeconds = 30;
Bounce addButton = Bounce(addButtonPin, 10);  // 10 ms debounce
Bounce subButton = Bounce(subButtonPin, 10);  // 10 ms debounce

volatile unsigned int count = 0;
volatile bool wasSleeping = false;
volatile bool waking = false;
unsigned long lastAction = 0;
volatile unsigned long lastWake = 0;

// All the one-time setup
void setup() {
  // Power saving
  ADCSRA = 0; // Disable A/D converter
  // Set all our unused pins to output to save power, skipping those in use
  for(int i = 4; i <= 19; i++) {
    pinMode(i, OUTPUT);
  }
  for(int i = 22; i <= 45; i++) {
    pinMode(i, OUTPUT);
  }
  
  lastAction = millis();  pinMode(addButtonPin, INPUT_PULLUP);
  pinMode(subButtonPin, INPUT_PULLUP);
  Serial.begin(57600);

  // 7 segment setup
  matrix.begin(0x70);
  matrix.setBrightness(8); // 0 to 15 (max)

  startupRoutine();

  attachInterrupt(addInterrupt, wakeUp, FALLING);
  attachInterrupt(subtractInterrupt, wakeUp, CHANGE); // This one is CHANGE because I like to mix it up?
}

// Main loop: check inputs, go to sleep if nothing is happening
void loop() {
  
  // If just waking from sleep, reset things
  if(waking) {
    Serial.begin(57600);
    startupRoutine();
    waking = false;
  }
  
  //Go to sleep: turn off display
  if(millis() - lastAction > wakeSeconds * 1000) {
    wasSleeping = true;
    analogWrite(addLedPin, 0);
    analogWrite(subLedPin, 0);
    matrix.clear();
    matrix.writeDisplay();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    noInterrupts(); // Don't interrupt me while I'm getting ready to sleep!
    sleep_enable(); // Pyjamas activated
    interrupts(); // Interrupts back on, otherwise we can't wake up
    sleep_cpu(); // Sleepies - we stop here until the interrupt happens
    sleep_disable(); // Pyjamas deactivated
  }
  
  // Check for inputs if fully woken up.
  if(!waking) {
    if (addButton.update()) {
      lastAction = millis(); // Update last button press to reset sleep timeout
      if (addButton.fallingEdge()) {
        analogWrite(addLedPin, 80); // Make the light bright when this button is pressed
        count = count + 1;
        matrix.println(count);
        matrix.writeDisplay();
        Serial.print("add falling\n"); // Debug output left for posterity
        
      } else {
        Serial.print("add rising\n");
        analogWrite(addLedPin, 20);
      }
    } 
  
    if (subButton.update()) {
      lastAction = millis();
      if (subButton.fallingEdge()) {
        analogWrite(subLedPin, 80);
        if(count > 0) {
          count = count - 1;
        } else if(millis() - lastWake > 1000) {
          matrix.clear(); // This whole thing is the zero moving from right to left and back again if we're already at zero
          matrix.writeDisplay();
          delay(100);
          matrix.writeDigitNum(4, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.clear();
          matrix.writeDigitNum(3, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.clear();
          matrix.writeDigitNum(1, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.clear();
          matrix.writeDigitNum(0, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.clear();
          matrix.writeDigitNum(1, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.clear();
          matrix.writeDigitNum(3, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.clear();
          matrix.writeDigitNum(4, 0);
          matrix.writeDisplay();
          delay(100);
          matrix.println(0);
          matrix.writeDisplay();
        }
        matrix.println(count);
        matrix.writeDisplay();
        Serial.print("sub falling\n");
        
      } else {
        Serial.print("sub rising\n");
        analogWrite(subLedPin, 20);
      }
    } 
  }
}

// Reset things when starting or waking
void startupRoutine() {
  count = 0;
  matrix.println(0);
  matrix.writeDisplay();

  analogWrite(addLedPin, 20);
  analogWrite(subLedPin, 20);
}

// Time for school!
void wakeUp() {
  if(wasSleeping) {
    lastAction = millis();
    lastWake = millis();
    wasSleeping = false;
    waking = true;
  }
}

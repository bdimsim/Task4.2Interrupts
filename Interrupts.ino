#define USING_TIMER_TC3 true // Must be placed before the header
#include "SAMDTimerInterrupt.h" // Can only use one timer
// #include <SAMD_ISR_Timer.h> // Can manage multiple timers

// byte (arduino specific) = uint8_t (c/c++ specific)
const byte sonarEchoPin = 7; // Not tolerant. Requires a voltage divider for exactly 3.3V
const byte sonarTriggerPin = 6; // 5V tolerant, accepts up to 5V
const byte ledPin = 5;
const byte interruptButtonPin = 3; // Button pin. Only pins 2, 3, 9, 10, 11, 13 support interrupts

const int DISTANCE_THRESHOLD = 10;

// volatile prevents compiler optimisations so we can update variables 
// with ISRs to properly reflect their values in the main loop
volatile byte ledState = LOW;

// Timer Counters (TC): used for general purpose time-based events.
// Timer Counter for Control (TCC): used for high precision tasks like PWM
SAMDTimer ITimer(TIMER_TC3); // TC is suitable for our regular period measurements

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptButtonPin, INPUT_PULLUP); // INPUT_PULLUP is HIGH by default
  pinMode(sonarTriggerPin, OUTPUT); // OUTPUT is LOW by default
  pinMode(sonarEchoPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptButtonPin), buttonToggleLED, CHANGE); // Pressing the button: HIGH --> LOW. Maybe try FALLING?
  ITimer.attachInterruptInterval_MS(100, sonarToggleLED); // Documentation recommends intervals over 60ms for echo readings
  Serial.begin(115200);
}

void loop() {
  digitalWrite(ledPin, ledState);
}

void buttonToggleLED() {
  static unsigned long last_interrupt_time = 0; // local static variables remember state even when the method call ends

  // Handles button debouncing for the push button
  // current time minus the time when the button was last pressed
  // If the bounce comes before 200ms, assume it's a bounce and ignore
  if (millis() - last_interrupt_time > 200) {
    ledState = !ledState;
    Serial.print(F("LED toggled via Push Button: "));
    Serial.println(ledState);
  }
  // Update the last interrupt time to the current time at which this method was called
  last_interrupt_time = millis();
}

void sonarToggleLED() {
  digitalWrite(sonarTriggerPin, HIGH); // Start pulse
  delayMicroseconds(10); // A 10us high pulse is needed to trigger the sensor
  digitalWrite(sonarTriggerPin, LOW); // End pulse
  float duration_us = pulseIn(sonarEchoPin, HIGH); // Read pulse through echo. Time in microseconds
  float distance_cm = 0.017 * duration_us; // Convert to distance in cm

  if(distance_cm <= DISTANCE_THRESHOLD) {
    ledState = !ledState;
    Serial.print(F("LED toggled via Ultrasonic Sensor: "));
    Serial.println(ledState);
  }
}

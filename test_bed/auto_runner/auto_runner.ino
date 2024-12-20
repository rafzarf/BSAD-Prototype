#include <AccelStepper.h>

// Define stepper motor connections and steps per revolution
#define dirPin 2
#define stepPin 4
#define stepsPerRevolution 800  // Adjust this value to match your stepper's specification

// Create an instance of the AccelStepper class
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

bool motorRunning = false;  // To track whether the motor is running
bool autoMode = false;      // To indicate if the system is in auto setup mode
unsigned long autoRunStartTime = 0; // Start time for automatic run
unsigned long autoRunDuration = 0;  // Duration for automatic run in milliseconds
unsigned long elapsedRunTime = 0;   // Elapsed time during the automatic run

// Limit switch pins
const int limitSwitchPin1 = 18; // GPIO pin 18 (backward limit)
const int limitSwitchPin2 = 19; // GPIO pin 19 (forward limit)

// Debounce variables
unsigned long debounceDelay = 50; // Debounce delay in milliseconds
unsigned long switch1LastPressTime = 0;
unsigned long switch2LastPressTime = 0;
bool switch1DebouncedState = HIGH;
bool switch2DebouncedState = HIGH;

// Step tracking variables
long stepCount = 0;  // Counter for steps
float distanceTraveled = 0.0;  // Distance traveled in millimeters
const float stepDistance = 0.00625;  // Distance per step based on ball screw pitch (5 mm / 200 steps)

void setup() {
  // Configure the limit switch pins as input with pull-up resistors
  pinMode(limitSwitchPin1, INPUT_PULLUP);
  pinMode(limitSwitchPin2, INPUT_PULLUP);
  
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);

  // Set initial conditions for the stepper motor
  stepper.setMaxSpeed(2000);  // Set maximum speed for continuous movement
  stepper.setAcceleration(1500);  // Acceleration for smooth starts and stops

  // Print instructions to the serial monitor
  Serial.println("Enter commands to control the stepper motor:");
  Serial.println("'f': move forward continuously");
  Serial.println("'b': move backward continuously");
  Serial.println("'1-9': adjust speed (1 is slow, 9 is fast)");
  Serial.println("'s': stop motor");
  Serial.println("'auto': prepare for automatic running");
}

void loop() {
  unsigned long currentTime = millis();

  // Check if the automatic run time has elapsed
  if (autoRunDuration > 0 && (currentTime - autoRunStartTime >= autoRunDuration)) {
    Serial.println("Automatic run completed. Stopping motor...");
    motorRunning = false;
    stepper.stop(); // Stop the stepper motor smoothly
    autoRunDuration = 0; // Reset automatic run duration
    displayRunSummary(); // Display summary at the end of the automatic run
  }

  // Debounce limit switch 1 (backward limit)
  int currentSwitch1State = digitalRead(limitSwitchPin1);
  if (currentSwitch1State != switch1DebouncedState) {
    if (currentTime - switch1LastPressTime > debounceDelay) {
      switch1DebouncedState = currentSwitch1State;
      switch1LastPressTime = currentTime;

      if (switch1DebouncedState == LOW) {
        Serial.println("Limit switch 1 hit! Stopping and moving forward immediately...");
        stepper.stop(); // Stop motor instantly
        stepper.setSpeed(1000); // Set speed for continuous forward movement
        motorRunning = true;
      }
    }
  }

  // Debounce limit switch 2 (forward limit)
  int currentSwitch2State = digitalRead(limitSwitchPin2);
  if (currentSwitch2State != switch2DebouncedState) {
    if (currentTime - switch2LastPressTime > debounceDelay) {
      switch2DebouncedState = currentSwitch2State;
      switch2LastPressTime = currentTime;

      if (switch2DebouncedState == LOW) {
        Serial.println("Limit switch 2 hit! Stopping and moving backward immediately...");
        stepper.stop(); // Stop motor instantly
        stepper.setSpeed(-1000); // Set speed for continuous backward movement
        motorRunning = true;
      }
    }
  }

  // Run the motor and track steps
  if (motorRunning) {
    stepper.runSpeed(); // Run the motor at a constant speed

    // Update step count and distance traveled
    static long lastStepPosition = 0;
    long currentStepPosition = stepper.currentPosition();
    
    if (currentStepPosition != lastStepPosition) {
      stepCount += abs(currentStepPosition - lastStepPosition);  // Increment step count
      distanceTraveled = stepCount * stepDistance;  // Calculate distance traveled
      lastStepPosition = currentStepPosition;

      // Print step count and distance traveled to the serial monitor periodically
      if (millis() % 5000 < 50) {  // Every 5 seconds
        elapsedRunTime = (millis() - autoRunStartTime) / 1000;  // Elapsed time in seconds
        Serial.print("Elapsed Time: ");
        Serial.print(elapsedRunTime / 3600.0, 2);  // Print in hours with 2 decimal places
        Serial.print(" hours, Steps: ");
        Serial.print(stepCount);
        Serial.print(", Distance Traveled: ");
        Serial.print(distanceTraveled);
        Serial.println(" mm");
      }
    }
  }

  // Handle manual control (Serial Commands)
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    handleManualControl(command);
  }
}

// Function to handle manual control from the serial monitor
void handleManualControl(String command) {
  command.trim();  // Remove any newline or whitespace characters

  if (command == "f") {
    Serial.println("Manual command: Moving forward continuously...");
    stepper.setSpeed(1000); // Set speed for continuous forward movement
    motorRunning = true;
  } else if (command == "b") {
    Serial.println("Manual command: Moving backward continuously...");
    stepper.setSpeed(-1000); // Set speed for continuous backward movement
    motorRunning = true;
  } else if (command == "s") {
    Serial.println("Manual command: Stopping motor...");
    motorRunning = false;  // Stop the motor
    stepper.stop(); // Stop the stepper motor smoothly
  } else if (command == "auto") {
    Serial.println("Preparing for automatic running. Enter duration in hours:");
    autoMode = true;  // Set the system to expect a duration input next
  } else if (autoMode) {
    int hours = command.toInt();
    if (hours > 0) {
      Serial.print("Running automatically for ");
      Serial.print(hours);
      Serial.println(" hour(s)...");
      autoRunDuration = hours * 3600000UL; // Convert hours to milliseconds
      autoRunStartTime = millis();
      stepper.setSpeed(1000); // Set speed for continuous forward movement during automatic run
      motorRunning = true;
      autoMode = false;  // Reset auto mode after setting duration
    } else {
      Serial.println("Invalid duration. Please enter a valid number of hours.");
    }
  } else if (command.length() == 1 && isDigit(command[0])) {
    int speed = map(command[0] - '0', 1, 9, 200, 2000);  // Map speed to a range (slow to fast)
    stepper.setMaxSpeed(speed);
    stepper.setSpeed((stepper.speed() > 0 ? speed : -speed)); // Adjust the current speed direction
    Serial.print("Speed set to ");
    Serial.println(speed);
  } else {
    Serial.println("Invalid command. Use 'f', 'b', 's', 'auto', or '1-9' to control the motor.");
  }
}

// Function to display the summary of the run
void displayRunSummary() {
  Serial.println("=== Run Summary ===");
  Serial.print("Total Steps: ");
  Serial.println(stepCount);
  Serial.print("Total Distance Traveled: ");
  Serial.print(distanceTraveled);
  Serial.println(" mm");
  Serial.print("Total Time Elapsed: ");
  Serial.print(elapsedRunTime / 3600.0, 2);  // Print in hours with 2 decimal places
  Serial.println(" hours");
}

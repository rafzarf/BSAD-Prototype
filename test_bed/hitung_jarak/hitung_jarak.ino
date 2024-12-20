// Define the pins connected to the limit switches
const int limitSwitchPin1 = 18; // GPIO pin 18
const int limitSwitchPin2 = 19; // GPIO pin 19

// Define stepper motor connections and steps per revolution
#define dirPin 2
#define stepPin 4
#define stepsPerRevolution 800  // Set to match your stepper's steps per revolution

int stepDelay = 400;  // Step delay in microseconds (adjustable via commands)
bool motorRunning = false;  // Indicates if the motor is currently running
bool automaticMode = false; // Indicates if automatic mode is active
bool returningToOrigin = false; // Indicates if the motor is in return mode
unsigned long lastStepTime = 0;  // Tracks the timing of each step

// Variables to track steps and distance
long stepCount = 0;  // Counter for steps
float distanceTraveled = 0.0;  // Distance traveled in millimeters
float actualDistance = 0.0;  // Manually measured actual distance
const float stepDistance = 0.00625;  // Distance per step based on ball screw pitch (5 mm / 200 steps)
long targetSteps = 0;  // Target number of steps for a given distance
const long bounceSteps = 100; // Number of steps for bounce

// Debounce variables
unsigned long debounceDelay = 50;  // Debounce delay in milliseconds
bool lastSwitch1State = HIGH;  // Last stable state of switch 1
bool lastSwitch2State = HIGH;  // Last stable state of switch 2
bool switch1State = HIGH;  // Current state of switch 1
bool switch2State = HIGH;  // Current state of switch 2

// Variables for timed mode
unsigned long startTime = 0;
unsigned long runDuration = 0;

void setup() {
  // Set pin modes
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(limitSwitchPin1, INPUT_PULLUP); // Limit switches use pull-up resistors
  pinMode(limitSwitchPin2, INPUT_PULLUP);

  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  Serial.println("Stepper Motor Control Initialized");
  Serial.println("Commands: move <distance>mm, f (forward), b (backward), s (stop), return, auto, 1h, 2h, 3h, 1-9 (speed)");
}

void loop() {
  // Read the limit switches with debouncing
  int currentSwitch1State = digitalRead(limitSwitchPin1);
  int currentSwitch2State = digitalRead(limitSwitchPin2);

  if (currentSwitch1State != lastSwitch1State) {
    delay(debounceDelay);  // Delay for debounce
    currentSwitch1State = digitalRead(limitSwitchPin1);  // Read the switch again

    if (currentSwitch1State != lastSwitch1State) {
      switch1State = currentSwitch1State;  // Update state if changed
      Serial.println("Limit switch 1 state changed.");
      if (switch1State == LOW) {
        // If we are returning to the origin or in automatic mode, stop the motor
        if (returningToOrigin || automaticMode) {
          motorRunning = false;
          automaticMode = false; // Stop automatic mode
          returningToOrigin = false;
          Serial.println("Motor stopped due to limit switch 1.");
        }
        resetDistance();
        Serial.println("Distance reset due to limit switch 1.");
        bounceForward();
      }
    }
  }
  lastSwitch1State = currentSwitch1State;

  if (currentSwitch2State != lastSwitch2State) {
    delay(debounceDelay);  // Delay for debounce
    currentSwitch2State = digitalRead(limitSwitchPin2);  // Read the switch again

    if (currentSwitch2State != lastSwitch2State) {
      switch2State = currentSwitch2State;  // Update state if changed
      Serial.println("Limit switch 2 state changed.");
    }
  }
  lastSwitch2State = currentSwitch2State;

  // Handle timed mode
  if (runDuration > 0 && (millis() - startTime >= runDuration)) {
    motorRunning = false;
    runDuration = 0;  // Reset run duration
    Serial.println("Timed run completed.");
  }

  // Handle motor movement if running
  if (motorRunning) {
    unsigned long currentMicros = micros();
    if (currentMicros - lastStepTime >= stepDelay) {
      // Perform a step
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(10); // Brief pulse
      digitalWrite(stepPin, LOW);
      lastStepTime = currentMicros;  // Update last step time

      // Update step count and distance
      stepCount++;  // Increment step count
      distanceTraveled += stepDistance;  // Increment the distance traveled

      // Check if target distance has been reached
      if (stepCount >= targetSteps && targetSteps > 0) {
        motorRunning = false;
        Serial.println("Target distance reached.");
      }

      // Print current step count and distance for monitoring
      Serial.print("Steps: ");
      Serial.print(stepCount);
      Serial.print(" | Calculated distance: ");
      Serial.print(distanceTraveled);
      Serial.println(" mm");
    }
  }

  // Check for serial input
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    handleCommand(command);
  }
}

// Function to handle serial commands
void handleCommand(String command) {
  command.trim(); // Remove any whitespace

  if (command.startsWith("move")) {
    // Parse distance command, expecting format like "move 10mm"
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex > 0) {
      float distanceToMove = command.substring(spaceIndex + 1, command.length() - 2).toFloat();
      if (distanceToMove > 0) {
        targetSteps = distanceToMove / stepDistance; // Calculate target steps
        stepCount = 0;  // Reset step count
        distanceTraveled = 0.0;  // Reset distance traveled
        motorRunning = true;
        Serial.print("Moving ");
        Serial.print(distanceToMove);
        Serial.println(" mm...");
      } else {
        Serial.println("Invalid distance.");
      }
    } else {
      Serial.println("Invalid move command. Use format: move <distance>mm");
    }
  } else if (command == "f") {
    // Move forward
    digitalWrite(dirPin, HIGH);
    targetSteps = 0;  // Clear target steps for continuous running
    stepCount = 0;  // Reset step count
    distanceTraveled = 0.0;  // Reset distance traveled
    motorRunning = true;
    Serial.println("Moving forward...");
  } else if (command == "b") {
    // Move backward
    digitalWrite(dirPin, LOW);
    targetSteps = 0;  // Clear target steps for continuous running
    stepCount = 0;  // Reset step count
    distanceTraveled = 0.0;  // Reset distance traveled
    motorRunning = true;
    Serial.println("Moving backward...");
  } else if (command == "s") {
    // Stop motor
    motorRunning = false;
    automaticMode = false; // Stop automatic mode if running
    Serial.println("Motor stopped.");
  } else if (command == "return") {
    // Return to the origin (limit switch 1)
    digitalWrite(dirPin, LOW);
    returningToOrigin = true;
    motorRunning = true;
    Serial.println("Returning to origin...");
  } else if (command == "auto") {
    // Activate automatic mode
    automaticMode = true;
    motorRunning = true;
    digitalWrite(dirPin, HIGH); // Set direction forward
    Serial.println("Automatic mode activated.");
  } else if (command == "1h" || command == "2h" || command == "3h") {
    // Timed run for 1, 2, or 3 hours
    int hours = command[0] - '0';  // Extract the number of hours
    runDuration = hours * 3600000UL;  // Convert hours to milliseconds
    startTime = millis();  // Set the start time
    motorRunning = true;
    digitalWrite(dirPin, HIGH); // Set direction forward
    Serial.print("Running for ");
    Serial.print(hours);
    Serial.println(" hour(s)...");
  } else if (command.length() == 1 && command[0] >= '1' && command[0] <= '9') {
    // Speed adjustment
    stepDelay = map(command[0] - '0', 1, 9, 1000, 200); // Map speed to delay range
    Serial.print("Speed set to ");
    Serial.println(command[0]);
  } else {
    Serial.println("Unknown command.");
  }
}

// Function to reset the distance and step count
void resetDistance() {
  stepCount = 0;
  distanceTraveled = 0.0;
  actualDistance = 0.0;
}

// Function to perform a bounce forward
void bounceForward() {
  Serial.println("Bouncing forward...");

  // Set direction to forward
  digitalWrite(dirPin, HIGH);

  // Perform a small number of steps for the bounce
  for (long i = 0; i < bounceSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  Serial.println("Bounce forward completed.");
}

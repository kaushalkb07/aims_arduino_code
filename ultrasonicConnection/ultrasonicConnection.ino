// Pin definitions
const int trigPin1 = 9;
const int echoPin1 = 10;
const int trigPin2 = 11;
const int echoPin2 = 12;
const int bulbPin1 = 2;  // Entry indication
const int bulbPin2 = 7;  // Exit indication
const int buzzerPin = 4; // Buzzer indication (connected to I/O pin)

// Distance variables
long duration1, distance1;
long duration2, distance2;

// Object tracking variables
int objectCounter = 1;
bool waitingForSensor2 = false;
bool waitingForSensor1 = false;

void setup() {
  Serial.begin(115200);

  // Set pin modes
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(bulbPin1, OUTPUT);
  pinMode(bulbPin2, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Initial bulb flash and buzzer beep to indicate the system is ready
  systemReadySignal();
}

void loop() {
  distance1 = getDistance(trigPin1, echoPin1);
  distance2 = getDistance(trigPin2, echoPin2);

  if (!waitingForSensor2 && !waitingForSensor1) {
    if (distance1 > 0 && distance1 < 3) {
      Serial.println("Sensor 1 detected an object. Ready to enter.");
      waitingForSensor2 = true;  // Waiting for Sensor 2 to confirm entry
      delay(1000);
    } else if (distance2 > 0 && distance2 < 5) {
      Serial.println("Sensor 2 detected an object. Ready to exit.");
      waitingForSensor1 = true;  // Waiting for Sensor 1 to confirm exit
      delay(1000);
    }
  }

  if (waitingForSensor2 && distance2 > 0 && distance2 < 5) {
    Serial.print("Object ");
    Serial.print(objectCounter);
    Serial.println(" has entered.");

    // Blink bulb 1 and sound buzzer twice for entry indication
    blinkBulbAndBuzzer(bulbPin1, 2);

    objectCounter++;
    waitingForSensor2 = false;
    delay(100);
  } else if (waitingForSensor1 && distance1 > 0 && distance1 < 5) {
    Serial.print("Object ");
    Serial.print(objectCounter);
    Serial.println(" has exited.");

    // Blink bulb 2 and sound buzzer twice for exit indication
    blinkBulbAndBuzzer(bulbPin2, 2);

    objectCounter++;
    waitingForSensor1 = false;
    delay(100);
  }

  delay(100);
}

// Function to measure distance
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout to avoid long waits
  long distance = duration * 0.034 / 2;
  return (duration == 0) ? -1 : distance;  // Return -1 if no pulse is received
}

// Function to blink a bulb and sound the buzzer multiple times
void blinkBulbAndBuzzer(int bulbPin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(bulbPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(bulbPin, LOW);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  }
}

// Function to indicate the system is ready (blink bulbs and beep buzzer once)
void systemReadySignal() {
  digitalWrite(bulbPin1, HIGH);
  digitalWrite(bulbPin2, HIGH);
  digitalWrite(buzzerPin, HIGH);
  delay(500);
  digitalWrite(bulbPin1, LOW);
  digitalWrite(bulbPin2, LOW);
  digitalWrite(buzzerPin, LOW);
  delay(500);
}

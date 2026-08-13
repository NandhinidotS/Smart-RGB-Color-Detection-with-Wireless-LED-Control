#include <EEPROM.h>
#include <SoftwareSerial.h>

// HC-05
// Arduino D10 = RX
// Arduino D11 = TX
SoftwareSerial Bluetooth(10, 11);

// TCS3200
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define SENSOR_OUT 8

// LEDs
#define RED_LED 12
#define GREEN_LED 13
#define WHITE_LED 3

// EEPROM
#define MAGIC_ADDRESS 0
#define MAGIC_VALUE 0x55AA

#define RED_R_ADDRESS    4
#define RED_G_ADDRESS    8
#define RED_B_ADDRESS    12

#define GREEN_R_ADDRESS  16
#define GREEN_G_ADDRESS  20
#define GREEN_B_ADDRESS  24

#define BLUE_R_ADDRESS   28
#define BLUE_G_ADDRESS   32
#define BLUE_B_ADDRESS   36

// CALIBRATION DATA
float redR;
float redG;
float redB;

float greenR;
float greenG;
float greenB;

float blueR;
float blueG;
float blueB;

// CALIBRATION SETTINGS
#define SAMPLE_COUNT 10

bool calibrationMode = false;

int calibrationStage = 0;

// 0 = RED
// 1 = GREEN
// 2 = BLUE
// 3 = COMPLETE

// SYSTEM MODE
// true  = color sensor controls LEDs
// false = Bluetooth controls LEDs
bool sensorControl = true;

// DETECTION INTERVAL
unsigned long previousDetection = 0;

#define DETECTION_INTERVAL 500

// SETUP
void setup()
{
  Serial.begin(9600);
  Bluetooth.begin(9600);

  // TCS3200
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(SENSOR_OUT, INPUT);

  // LEDs
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);

  // All LEDs OFF
  turnOffAllLEDs();

  // TCS3200 frequency scaling
  // 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println();
  Serial.println("TCS3200 RGB COLOR DETECTION SYSTEM");

  Bluetooth.println();
  Bluetooth.println("RGB COLOR DETECTION SYSTEM");

  // CHECK EEPROM
  if (loadCalibration())
  {
    Serial.println();
    Serial.println("Calibration found in EEPROM.");
    Serial.println("Starting sensor detection...");

    Bluetooth.println("Calibration loaded.");
    Bluetooth.println("SENSOR MODE");

    calibrationMode = false;
    sensorControl = true;
  }
  else
  {
    Serial.println();
    Serial.println("No calibration found.");

    Bluetooth.println("No calibration found.");

    startCalibration();
  }
}

// LOOP
void loop()
{
  // CALIBRATION
  if (calibrationMode)
  {
    checkEnter();
    return;
  }

  // BLUETOOTH
  checkBluetooth();

  // COLOR SENSOR CONTROL
  if (sensorControl)
  {
    if (millis() - previousDetection >= DETECTION_INTERVAL)
    {
      previousDetection = millis();

      detectColor();
    }
  }
}

// START CALIBRATION
void startCalibration()
{
  calibrationMode = true;
  calibrationStage = 0;

  Serial.println();
  Serial.println("RGB CALIBRATION");

  Serial.println("Place RED object.");
  Serial.println("Press ENTER.");

  Bluetooth.println();
  Bluetooth.println("RGB CALIBRATION");
  Bluetooth.println("Place RED object.");
  Bluetooth.println("Press ENTER.");
}

// CHECK ENTER
void checkEnter()
{
  if (Serial.available() > 0)
  {
    char received = Serial.read();

    if (received == '\n' || received == '\r')
    {
      // Remove remaining CR/LF
      delay(50);

      while (Serial.available() > 0)
      {
        Serial.read();
      }

      calibrateCurrentColor();
    }
  }
}

// CALIBRATE CURRENT COLOR
void calibrateCurrentColor()
{
  float averageR;
  float averageG;
  float averageB;

  // RED
  if (calibrationStage == 0)
  {
    Serial.println();
    Serial.println("RED CALIBRATION STARTED");

    Bluetooth.println("RED CALIBRATION STARTED");

    delay(500);

    read10Samples(
      averageR,
      averageG,
      averageB
    );

    redR = averageR;
    redG = averageG;
    redB = averageB;

    saveRedCalibration();

    Serial.println();
    Serial.println("RED CALIBRATION SAVED");

    printCalibration(
      "RED",
      redR,
      redG,
      redB
    );

    Bluetooth.println("RED SAVED");

    calibrationStage = 1;

    Serial.println();
    Serial.println("Place GREEN object.");
    Serial.println("Press ENTER.");

    Bluetooth.println("Place GREEN object.");
    Bluetooth.println("Press ENTER.");

    return;
  }

  // GREEN
  if (calibrationStage == 1)
  {
    Serial.println();
    Serial.println("GREEN CALIBRATION STARTED");

    Bluetooth.println("GREEN CALIBRATION STARTED");

    delay(500);

    read10Samples(
      averageR,
      averageG,
      averageB
    );

    greenR = averageR;
    greenG = averageG;
    greenB = averageB;

    saveGreenCalibration();

    Serial.println();
    Serial.println("GREEN CALIBRATION SAVED");

    printCalibration(
      "GREEN",
      greenR,
      greenG,
      greenB
    );

    Bluetooth.println("GREEN SAVED");

    calibrationStage = 2;

    Serial.println();
    Serial.println("Place BLUE object.");
    Serial.println("Press ENTER.");

    Bluetooth.println("Place BLUE object.");
    Bluetooth.println("Press ENTER.");

    return;
  }

  // BLUE
  if (calibrationStage == 2)
  {
    Serial.println();
    Serial.println("BLUE CALIBRATION STARTED");

    Bluetooth.println("BLUE CALIBRATION STARTED");

    delay(500);

    read10Samples(
      averageR,
      averageG,
      averageB
    );

    blueR = averageR;
    blueG = averageG;
    blueB = averageB;

    saveBlueCalibration();

    Serial.println();
    Serial.println("BLUE CALIBRATION SAVED");

    printCalibration(
      "BLUE",
      blueR,
      blueG,
      blueB
    );

    Bluetooth.println("BLUE SAVED");

    // Finish
    calibrationStage = 3;

    EEPROM.put(
      MAGIC_ADDRESS,
      MAGIC_VALUE
    );

    calibrationMode = false;
    sensorControl = true;

    turnOffAllLEDs();

    Serial.println();
    Serial.println("CALIBRATION COMPLETE");

    Serial.println("Automatic color detection started.");

    Bluetooth.println();
    Bluetooth.println("CALIBRATION COMPLETE");
    Bluetooth.println("SENSOR MODE STARTED.");

    delay(1000);

    return;
  }
}

// TAKE 10 SAMPLES
void read10Samples(
  float &averageR,
  float &averageG,
  float &averageB
)
{
  float totalR = 0;
  float totalG = 0;
  float totalB = 0;

  Serial.println();
  Serial.println("Taking 10 samples...");

  Bluetooth.println("Taking 10 samples...");

  for (int sample = 1; sample <= SAMPLE_COUNT; sample++)
  {
    float r;
    float g;
    float b;

    readRGB(
      r,
      g,
      b
    );

    totalR += r;
    totalG += g;
    totalB += b;

    Serial.print("Sample ");
    Serial.print(sample);

    Serial.print("  R=");
    Serial.print(r, 4);

    Serial.print("  G=");
    Serial.print(g, 4);

    Serial.print("  B=");
    Serial.println(b, 4);

    delay(100);
  }

  averageR = totalR / SAMPLE_COUNT;
  averageG = totalG / SAMPLE_COUNT;
  averageB = totalB / SAMPLE_COUNT;

  Serial.println();
  Serial.println("10-SAMPLE AVERAGE");

  Serial.print("R = ");
  Serial.println(averageR, 4);

  Serial.print("G = ");
  Serial.println(averageG, 4);

  Serial.print("B = ");
  Serial.println(averageB, 4);
}

// READ TCS3200 RGB
void readRGB(
  float &r,
  float &g,
  float &b
)
{
  unsigned long rawR;
  unsigned long rawG;
  unsigned long rawB;

  // RED
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  delay(20);

  rawR = pulseIn(
    SENSOR_OUT,
    LOW,
    100000
  );

  // GREEN
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);

  delay(20);

  rawG = pulseIn(
    SENSOR_OUT,
    LOW,
    100000
  );

  // BLUE
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);

  delay(20);

  rawB = pulseIn(
    SENSOR_OUT,
    LOW,
    100000
  );

  // Avoid zero
  if (rawR == 0)
    rawR = 1;

  if (rawG == 0)
    rawG = 1;

  if (rawB == 0)
    rawB = 1;

  // INVERSE FREQUENCY
  float inverseR = 1.0 / (float)rawR;
  float inverseG = 1.0 / (float)rawG;
  float inverseB = 1.0 / (float)rawB;

  float total =
    inverseR +
    inverseG +
    inverseB;

  // NORMALIZED RGB
  r = inverseR / total;
  g = inverseG / total;
  b = inverseB / total;
}

// COLOR DETECTION
void detectColor()
{
  float r;
  float g;
  float b;

  readRGB(
    r,
    g,
    b
  );

  // DISTANCES
  float redDistance =
    calculateDistance(
      r,
      g,
      b,
      redR,
      redG,
      redB
    );

  float greenDistance =
    calculateDistance(
      r,
      g,
      b,
      greenR,
      greenG,
      greenB
    );

  float blueDistance =
    calculateDistance(
      r,
      g,
      b,
      blueR,
      blueG,
      blueB
    );

  // FIND BEST COLOR
  float bestDistance;
  float secondDistance;

  String detectedColor;

  if (
    redDistance <= greenDistance &&
    redDistance <= blueDistance
  )
  {
    bestDistance = redDistance;
    detectedColor = "RED";

    if (greenDistance < blueDistance)
      secondDistance = greenDistance;
    else
      secondDistance = blueDistance;
  }
  else if (
    greenDistance <= redDistance &&
    greenDistance <= blueDistance
  )
  {
    bestDistance = greenDistance;
    detectedColor = "GREEN";

    if (redDistance < blueDistance)
      secondDistance = redDistance;
    else
      secondDistance = blueDistance;
  }
  else
  {
    bestDistance = blueDistance;
    detectedColor = "BLUE";

    if (redDistance < greenDistance)
      secondDistance = redDistance;
    else
      secondDistance = greenDistance;
  }

  float separation =
    secondDistance - bestDistance;

  // SERIAL DATA
  Serial.print("R=");
  Serial.print(r, 3);

  Serial.print(" G=");
  Serial.print(g, 3);

  Serial.print(" B=");
  Serial.print(b, 3);

  Serial.print(" | RED D=");
  Serial.print(redDistance, 3);

  Serial.print(" | GREEN D=");
  Serial.print(greenDistance, 3);

  Serial.print(" | BLUE D=");
  Serial.print(blueDistance, 3);

  Serial.print(" | Separation=");
  Serial.println(separation, 3);

  // ACCEPT COLOR
  if (
    bestDistance < 0.10 &&
    separation > 0.02
  )
  {
    Serial.print("RESULT = ");
    Serial.print(detectedColor);
    Serial.println(" DETECTED");

    Bluetooth.print("RESULT = ");
    Bluetooth.print(detectedColor);
    Bluetooth.println(" DETECTED");

    // LED CONTROL
    if (detectedColor == "RED")
    {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(WHITE_LED, LOW);
    }
    else if (detectedColor == "GREEN")
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(WHITE_LED, LOW);
    }
    else if (detectedColor == "BLUE")
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(WHITE_LED, HIGH);
    }
  }
  else
  {
    Serial.println("RESULT = UNKNOWN COLOR DETECTED");

    Bluetooth.println("RESULT = UNKNOWN COLOR DETECTED");

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(WHITE_LED, LOW);
  }
}

// DISTANCE
float calculateDistance(
  float r1,
  float g1,
  float b1,
  float r2,
  float g2,
  float b2
)
{
  float dr = r1 - r2;
  float dg = g1 - g2;
  float db = b1 - b2;

  return sqrt(
    dr * dr +
    dg * dg +
    db * db
  );
}

// BLUETOOTH CONTROL
void checkBluetooth()
{
  if (Bluetooth.available() == 0)
    return;

  char command = Bluetooth.read();

  // Convert lowercase to uppercase
  if (
    command >= 'a' &&
    command <= 'z'
  )
  {
    command = command - 32;
  }

  // C = COLOR SENSOR MODE
  if (command == 'C')
  {
    sensorControl = true;

    turnOffAllLEDs();

    Bluetooth.println("SENSOR CONTROL MODE");
    Serial.println("SENSOR CONTROL MODE");

    return;
  }

  // R = RED LED
  if (command == 'R')
  {
    sensorControl = false;

    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(WHITE_LED, LOW);

    Bluetooth.println("RED LED ON");

    return;
  }

  // G = GREEN LED
  if (command == 'G')
  {
    sensorControl = false;

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(WHITE_LED, LOW);

    Bluetooth.println("GREEN LED ON");

    return;
  }

  // B = BLUE INDICATOR
  // White LED represents BLUE
  if (command == 'B')
  {
    sensorControl = false;

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);

    Bluetooth.println("WHITE LED ON - BLUE INDICATOR");

    return;
  }

  // A = ALL LEDs
  if (command == 'A')
  {
    sensorControl = false;

    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(WHITE_LED, HIGH);

    Bluetooth.println("ALL LEDs ON");

    return;
  }

  // O = OFF
  if (command == 'O')
  {
    sensorControl = false;

    turnOffAllLEDs();

    Bluetooth.println("ALL LEDs OFF");

    return;
  }
}

// ALL LEDS OFF
void turnOffAllLEDs()
{
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(WHITE_LED, LOW);
}

// SAVE RED
void saveRedCalibration()
{
  EEPROM.put(
    RED_R_ADDRESS,
    redR
  );

  EEPROM.put(
    RED_G_ADDRESS,
    redG
  );

  EEPROM.put(
    RED_B_ADDRESS,
    redB
  );
}

// SAVE GREEN
void saveGreenCalibration()
{
  EEPROM.put(
    GREEN_R_ADDRESS,
    greenR
  );

  EEPROM.put(
    GREEN_G_ADDRESS,
    greenG
  );

  EEPROM.put(
    GREEN_B_ADDRESS,
    greenB
  );
}

// SAVE BLUE
void saveBlueCalibration()
{
  EEPROM.put(
    BLUE_R_ADDRESS,
    blueR
  );

  EEPROM.put(
    BLUE_G_ADDRESS,
    blueG
  );

  EEPROM.put(
    BLUE_B_ADDRESS,
    blueB
  );
}

// LOAD EEPROM
bool loadCalibration()
{
  unsigned int magic;

  EEPROM.get(
    MAGIC_ADDRESS,
    magic
  );

  if (magic != MAGIC_VALUE)
  {
    return false;
  }

  EEPROM.get(
    RED_R_ADDRESS,
    redR
  );

  EEPROM.get(
    RED_G_ADDRESS,
    redG
  );

  EEPROM.get(
    RED_B_ADDRESS,
    redB
  );

  EEPROM.get(
    GREEN_R_ADDRESS,
    greenR
  );

  EEPROM.get(
    GREEN_G_ADDRESS,
    greenG
  );

  EEPROM.get(
    GREEN_B_ADDRESS,
    greenB
  );

  EEPROM.get(
    BLUE_R_ADDRESS,
    blueR
  );

  EEPROM.get(
    BLUE_G_ADDRESS,
    blueG
  );

  EEPROM.get(
    BLUE_B_ADDRESS,
    blueB
  );

  // VALIDATION
  if (
    redR <= 0 ||
    redG <= 0 ||
    redB <= 0 ||
    greenR <= 0 ||
    greenG <= 0 ||
    greenB <= 0 ||
    blueR <= 0 ||
    blueG <= 0 ||
    blueB <= 0
  )
  {
    return false;
  }

  // Print stored values
  Serial.println();
  Serial.println("EEPROM CALIBRATION:");

  printCalibration(
    "RED",
    redR,
    redG,
    redB
  );

  printCalibration(
    "GREEN",
    greenR,
    greenG,
    greenB
  );

  printCalibration(
    "BLUE",
    blueR,
    blueG,
    blueB
  );

  return true;
}

// PRINT CALIBRATION
void printCalibration(
  String name,
  float r,
  float g,
  float b
)
{
  Serial.print(name);
  Serial.print(" : ");

  Serial.print(r, 4);
  Serial.print(" , ");

  Serial.print(g, 4);
  Serial.print(" , ");

  Serial.println(b, 4);
}

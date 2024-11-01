#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <TinyGPS++.h>

#define ACC_THRESHOLD 10 // Sensitivity threshold for accident detection
#define ALERT_PHONE_NUMBER "+1234567890" // Replace with the emergency contact number

// Define pins
const int accXPin = A0;    // ADXL335 Accelerometer X-axis
const int accYPin = A1;    // ADXL335 Accelerometer Y-axis
const int accZPin = A2;    // ADXL335 Accelerometer Z-axis
const int switchPin = 7;   // Switch for manual reset

// Initialize modules
SoftwareSerial gsmSerial(10, 11); // RX, TX for SIM900A GSM
SoftwareSerial gpsSerial(4, 3);   // RX, TX for SIM28ML GPS
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // LCD pins
TinyGPSPlus gps;

void setup() {
  // Set up serial communication
  Serial.begin(9600);
  gsmSerial.begin(9600);
  gpsSerial.begin(9600);

  // Initialize pins
  pinMode(accXPin, INPUT);
  pinMode(accYPin, INPUT);
  pinMode(accZPin, INPUT);
  pinMode(switchPin, INPUT_PULLUP); // Active LOW for reset

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Vehicle Alert");

  // GSM setup
  sendGSMCommand("AT");             // Check if GSM is responding
  sendGSMCommand("AT+CMGF=1");      // Set to text mode
  sendGSMCommand("AT+CNMI=1,2,0,0,0"); // Configure message display on arrival
  delay(1000);
}

void loop() {
  // Read accelerometer data
  int accX = analogRead(accXPin);
  int accY = analogRead(accYPin);
  int accZ = analogRead(accZPin);

  // Check for accident based on threshold
  if (accX > ACC_THRESHOLD || accY > ACC_THRESHOLD || accZ > ACC_THRESHOLD) {
    lcd.clear();
    lcd.print("Accident Detected");

    // Wait for GPS data
    String location = getGPSLocation();
    sendSMS(ALERT_PHONE_NUMBER, "Accident detected! Location: " + location);
  }

  // Check for reset
  if (digitalRead(switchPin) == LOW) {
    lcd.clear();
    lcd.print("System Reset");
    delay(1000); // Debounce delay
  }
  
  delay(500); // Delay for next reading
}

// Function to send commands to the GSM module
void sendGSMCommand(String command) {
  gsmSerial.println(command);
  delay(500); // Allow time for the GSM module to process
}

// Function to send SMS with the accident alert and GPS location
void sendSMS(String phoneNumber, String message) {
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(500);
  gsmSerial.print(message);
  gsmSerial.write(26); // ASCII code of CTRL+Z to send SMS
  delay(1000);
}

// Function to get GPS location as a string
String getGPSLocation() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      String lat = String(gps.location.lat(), 6); // Latitude with 6 decimal places
      String lng = String(gps.location.lng(), 6); // Longitude with 6 decimal places
      return "Lat: " + lat + ", Lon: " + lng;
    }
  }
  return "No GPS fix"; // Returns if no GPS fix available
}

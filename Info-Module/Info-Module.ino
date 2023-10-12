/* INFO MODULE ARDUINO 1*/
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#define PASSWORD_LENGTH 3
#define STATUS_LOGIN 0
#define STATUS_MAIN_MENU 1
#define STATUS_INPUT_OLD_PASS 2
#define STATUS_INPUT_NEW_PASS 3
#define STATUS_CONFIRM_NEW_PASS 4

const String prompts[5][2] = {
  { "LOGIN MENU", "Password: " },
  { "*#: Close vault", "**#: Change Pass" },
  { "Enter old pass:", "" },
  { "Enter new pass:", "" },
  { "Confirm new pass:", "" }
};


const int pingSensorPin = 12;
const int pirSensorPin = 11;
const byte rxPin = 8;
const byte txPin = 9;
bool lcdActive = false;
char lockerInfo;
int distance;
String input;
String tempPassword;
String storedPassword = "123";
int inputMode = 0;
unsigned long lastSendTime = 0;
unsigned long currentTime = 0;

SoftwareSerial InfoComms(rxPin, txPin);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);  // Pins for LCD

void setup() {
  Wire.begin();
  lcd.begin(16, 2);
  pinMode(pingSensorPin, INPUT);
  pinMode(pirSensorPin, INPUT);
  Serial.begin(9600);  // Inisialisasi komunikasi serial
  InfoComms.begin(4800);
}

void loop() {
  delay(50);

 int bytesReceived = 0;  // Inisialisasi bytesReceived ke 0

  // Wire communication with battery
  currentTime = millis();
  if (currentTime - lastSendTime > 1000) {
    Wire.beginTransmission(8);
    Wire.write("PWR");
    Wire.endTransmission();
    lastSendTime = currentTime;
    Serial.println("send request");
    Wire.requestFrom(8, 3);  // Request data from the slave
    bytesReceived = Wire.available();  // Update bytesReceived
  }
  
  if (bytesReceived > 0) {
    String responseString = "";
    while (Wire.available()) {
      char receivedChar = Wire.read();
      responseString += receivedChar;
    }
    Serial.println("Received: " + responseString);  // Print the received data
  }

  distance = readPingSensor();
  if (distance <= 50 && !lcdActive) {
    enableDisplay();
    displayPrompt(STATUS_LOGIN, true);
    enableKeypad();
    input = "";
  } else if (distance > 50 && lcdActive) {  // If LCD was on
    disableDisplay();
    disableKeypad();
  }

  if (digitalRead(pirSensorPin) == HIGH) {
    Serial.println("Warning: Ada orang disekitar brankas!");
  }

  if (InfoComms.available() == 0) {
    return;
  }

  lockerInfo = InfoComms.read();
  Serial.println(lockerInfo);

  // Check fire status before anything
  if (lockerInfo == '!') {
    Serial.println("Kebakaran!");
    closeLocker();
    disableKeypad();
    disableDisplay();
    return;
  }

  Serial.println("Mode: " + inputMode);
  // input mode: LOGIN
  if (inputMode == STATUS_LOGIN) {

    // Clear password input
    if (lockerInfo == '*') {
      input = "";
      displayPrompt(STATUS_LOGIN, true);

      // Enter password input
    } else if (lockerInfo == '#') {

      if (input == storedPassword) {
        inputMode = 4;
        lcd.clear();
        displayPrompt(STATUS_MAIN_MENU, false);
        input = "";
      } else {
        lcd.clear();
        lcd.print("Incorrect");
        lcd.setCursor(0, 1);
        lcd.print("Password!");
        delay(1000);
        displayPrompt(STATUS_LOGIN, true);
      }
      input = "";

      // Input Characters
    } else {
      lcd.print(lockerInfo);
      input += lockerInfo;
    }
  }
  // input mode: MAIN MENU
  else if (inputMode == STATUS_MAIN_MENU) {
    if (lockerInfo == '#') {

      // logic for closing vault
      if (input == "*") {
        input = "";
        inputMode = 0;
        disableDisplay();
        closeLocker();

        // Enter Password change mode
      } else if (input == "**") {
        displayPrompt(STATUS_INPUT_OLD_PASS, true);
        input = "";
      } else {
        lcd.clear();
        lcd.print("Not a valid");
        lcd.setCursor(0, 1);
        lcd.print("command");
        delay(1000);
        displayPrompt(STATUS_MAIN_MENU, false);
        input = "";
      }
    } else {
      if (input.length() == 0) {
        lcd.clear();
        lcd.blink();
      }
      lcd.print(lockerInfo);
      input += lockerInfo;
    }
  }
  // input mode: INPUT OLD PASSWORD
  else if (inputMode == STATUS_INPUT_OLD_PASS) {
    if (lockerInfo == '#') {
      if (input == storedPassword) {
        displayPrompt(STATUS_INPUT_NEW_PASS, true);
        input = "";

      } else {
        lcd.clear();
        lcd.print("Incorrect");
        lcd.setCursor(0, 1);
        lcd.print("Password!");
        delay(1000);
        displayPrompt(STATUS_MAIN_MENU, false);
        input = "";
      }
    } else if (lockerInfo == '*') {
      input = "";
      displayPrompt(STATUS_INPUT_OLD_PASS, false);
    } else {
      input += lockerInfo;
      lcd.print(lockerInfo);
    }
  }
  // input mode: INPUT NEW PASSWORD
  else if (inputMode == STATUS_INPUT_NEW_PASS) {
    if (lockerInfo == '#') {
      tempPassword = input;
      input = "";
      displayPrompt(STATUS_CONFIRM_NEW_PASS, true);
    } else if (lockerInfo == '*') {
      input = "";
      displayPrompt(STATUS_INPUT_NEW_PASS, false);
    } else {
      input += lockerInfo;
      lcd.print(lockerInfo);
    }
  }

  // input mode: CONFIRM NEW PASSWORD
  else if (inputMode == STATUS_CONFIRM_NEW_PASS) {
    if (lockerInfo == '#') {
      if (input == tempPassword) {
        storedPassword = input;
        input = "";
        displayPrompt(STATUS_MAIN_MENU, true);
      } else {
        lcd.clear();
        lcd.print("Password");
        lcd.setCursor(0, 1);
        lcd.print("doesn't match!");
        delay(1000);
        displayPrompt(STATUS_INPUT_NEW_PASS, true);
        input = "";
      }
    } else if (lockerInfo == '*') {
      input = "";
      displayPrompt(STATUS_CONFIRM_NEW_PASS, false);
    } else {
      input += lockerInfo;
      lcd.print(lockerInfo);
    }
  }
}

int readPingSensor() {
  long duration;
  int distance;

  pinMode(pingSensorPin, OUTPUT);
  digitalWrite(pingSensorPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingSensorPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingSensorPin, LOW);

  pinMode(pingSensorPin, INPUT);
  duration = pulseIn(pingSensorPin, HIGH);
  distance = duration / 29 / 2;

  return distance;
}

void displayPrompt(int promptNumber, bool blink) {
  inputMode = promptNumber;
  lcd.clear();
  lcd.print(prompts[promptNumber][0]);
  lcd.setCursor(0, 1);
  lcd.print(prompts[promptNumber][1]);
  if (blink) {
    lcd.blink();
  }
}

void disableDisplay() {
  lcd.noDisplay();  // Turn off LCD display
  Serial.println("LCD Off");
  lcdActive = false;
}
void enableDisplay() {
  lcd.begin(16, 2);  // Reinitialize LCD
  Serial.println("LCD On");
  lcdActive = true;
}

void closeLocker() {
  InfoComms.print('C');
}
void openLocker() {
  InfoComms.print('O');
}
void enableKeypad() {
  InfoComms.print('E');
}
void disableKeypad() {
  InfoComms.print('D');
}

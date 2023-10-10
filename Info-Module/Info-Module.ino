/************* Modul Info **************

   Modul Info ini bertugas untuk:
   1. Menampilkan informasi di layar LCD.
   2. Mengendalikan tampilan dan respon keypad sesuai
      kondisi sensor ping maupun sensor PIR
   3. Mengirim sinyal buka/tutup servo brankas (ke modul
      Locker)
   4. Membaca status kelistrikan sistem (dari modul Power)

   Jika orang berada dalam jarak yang cukup dekat (dideteksi
   oleh sensor PING sebagai jarak <= 50cm), maka LCD dinyalakan.
   Dan jika orang tersebut menjauh atau berada di luar 
   coverage area sensor PING, layar LCD dimatikan.
   Jika LCD dalam keadaan mati, maka input dari keypad SEHARUSNYA   
   tidak akan diproses!
   
   Jika sensor PING mendeteksi ada gerakan orang, kirimkan
   notifikasi via Serial monitor (misal Warning: "Ada orang
   disekitar brankas!")

   Setiap 1 detik sekali, Arduino "Info" akan mengirim request
   status kelistrikan dari Modul "Power". Komunikasi antara modul
   "Info" dengan modul "Power" adalah via I2C
   Gunakan kata kunci I2C masternya: "PWR"

   Khusus jika dia mendeteksi tegangan baterei kurang 
   dari 4.00V, maka kirimkan kode peringatan lewat Serial 
   monitor (misal Warning: "Baterei mau habis!").
   Dan JIKA Arduino Info ini menerima data berupa "0.0" dari
   modul Power, maka sistem secara keseluruhan akan di-shutdown 
   dan dia akan mengirim sinyal ke modul Locker untuk menutup 
   servo.
   
   Catatan: untuk SoftwareSerial, gunakan baudrate 4800
*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#define PASSWORD_LENGTH 3


LiquidCrystal lcd(7, 6, 5, 4, 3, 2);  // Pins for LCD
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

/*
STATUS MODES
- login				          0  
- input old password		1  logged in
- input new password		2  logged in
- confirm new password  3  logged in
- main menu			        4  logged in
*/
int inputMode = 0;


SoftwareSerial InfoComms(rxPin, txPin);

void setup() {
  lcd.begin(16, 2);
  pinMode(pingSensorPin, INPUT);
  pinMode(pirSensorPin, INPUT);
  Serial.begin(9600);  // Inisialisasi komunikasi serial
  InfoComms.begin(4800);
}

void loop() {
  delay(10);
  // TODO: setup wire communication with battery

  distance = readPingSensor();
  if (distance <= 50 && !lcdActive) {
    enableDisplay();
    displayLoginMenu();
    enableKeypad();
    input = "";

  } else {  // If LCD was on
    disableDisplay();
    disableKeypad();
  }

  if (digitalRead(pirSensorPin) == HIGH) {
    Serial.println("Warning: Ada orang disekitar brankas!");
  }


  if (InfoComms.available() == 0) { return; }

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


  // input mode: LOGIN
  if (inputMode == 0) {

    // Clear password input
    if (lockerInfo == '*') {
      input = "";
      displayLoginMenu();

      // Enter password input
    } else if (lockerInfo == '#') {

      if (input == storedPassword) {
        inputMode = 4;
        lcd.clear();
        displayMainMenu();
      } else {
        lcd.clear();
        lcd.print("Incorrect Password!");
        delay(1000);
        displayLoginMenu();
      }
      input = "";

      // Input Characters
    } else {
      lcd.print(lockerInfo);
      input += lockerInfo;
    }


  }
  // input mode: INPUT OLD PASSWORD
  else if (inputMode == 1) {
    if (input.length() < (PASSWORD_LENGTH - 1)) {
      input += lockerInfo;
    } else {
      input += lockerInfo;
      if (storedPassword.equals(input)) {
        lcd.clear();
        lcd.print("New pass:");
        lcd.setCursor(0, 1);
        inputMode = 2;
        input = "";
      }
    }

  }
  // input mode: INPUT NEW PASSWORD
  else if (inputMode == 2) {
    if (input.length() < (PASSWORD_LENGTH - 1)) {
      input += lockerInfo;
    } else {
      input += lockerInfo;
      lcd.clear();
      lcd.print("Confirm pass:");
      lcd.setCursor(0, 1);
      inputMode = 3;
      tempPassword = input;
      input = "";
    }

  }
  // input mode: CONFIRM NEW PASSWORD
  else if (inputMode == 3) {
    if (input.length() < (PASSWORD_LENGTH - 1)) {
      input += lockerInfo;
    } else {
      input += lockerInfo;
      if (storedPassword.equals(input)) {
        lcd.clear();
        lcd.print("Password change");
        lcd.setCursor(0,1);
        lcd.print("Success!");
        delay(1000);
        displayMainMenu();
        inputMode = 4;
        input = "";
      }
    }

  }
  // input mode: MAIN MENU
  else if (inputMode == 4) {
    if (lockerInfo == '#') {

      // logic for closing vault
      if (input == "*") {
        input = "";
        inputMode = 0;
        disableDisplay();
        closeLocker();

        // Enter Password change mode
      } else if (input == "**") {
        lcd.print("Enter old pass:");
        lcd.setCursor(0, 1);
        inputMode = 1;
        input = "";

      } else {
        lcd.clear();
        lcd.print("Not valid command");
        delay(500);
        displayMainMenu();
        input = "";
      }

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

void displayLoginMenu() {
  lcd.cursor();
  lcd.blink();
  lcd.clear();
  lcd.print("LOGIN MENU");
  lcd.setCursor(0, 1);
  lcd.print("Password: ");
}
void displayMainMenu() {
  lcd.clear();
  lcd.print("*#: Close vault");
  lcd.setCursor(0, 1);
  lcd.print("**#: Change Pass");
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

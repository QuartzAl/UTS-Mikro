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
#define STORED_PASSWORD "123"

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);  // Pins for LCD
const int pingSensorPin = 12;
const int pirSensorPin = 11;
const byte rxPin = 8;
const byte txPin = 9;
bool lcdActive = false;
bool loggedIn;
bool inputMode = false;
char keypadPress;
int distance;
String input;

SoftwareSerial InfoComms(rxPin, txPin);

void setup() {
  lcd.begin(16, 2);
  pinMode(pingSensorPin, INPUT);
  pinMode(pirSensorPin, INPUT);
  Serial.begin(9600);  // Inisialisasi komunikasi serial
  InfoComms.begin(4800);
}

void loop() {
  // TODO: setup wire communication with battery
  // TODO: Check fire status before anything
  

  if (loggedIn) {
    keypadPress = InfoComms.read();
    if (keypadPress == '#'){

      // logic for closing vault
      if (input == "*"){
        input = "";
        loggedIn = false;
        lcdActive = false;
        closeLocker();
        lcd.clear();
      }else if (input == "**"){

        // TODO: set system for changing password
      }else{
        lcd.clear();
        lcd.print("Not valid command");
        delay(500);
        lcd.clear();
        lcd.print("*#: Close vault");
        lcd.setCursor(0,1);
        lcd.print("**#: Change Pass");
        input = "";
      }
    }else{
      if(!inputMode){
        lcd.clear();
        input = "";
        inputMode = true;
      }
      input += keypadPress;
      lcd.print(keypadPress);

    }

  } else {
    distance = readPingSensor();
    if (distance <= 50) {
      if (!lcdActive) {    // If LCD was off
        lcd.begin(16, 2);  // Reinitialize LCD
        Serial.println("LCD On");
        displayLoginMenu();
        input = "";
        enableKeypad();
        lcdActive = true;
      }

      if (InfoComms.available() > 0) {
        keypadPress = InfoComms.read();
        Serial.println(keypadPress);
        if (keypadPress == '*') {
          input = "";
          displayLoginMenu();

        } else if (keypadPress == '#') {
          if (input == STORED_PASSWORD) {
            loggedIn = true;
            input = "";
            lcd.clear();
            lcd.println("*#: Close vault");
            lcd.print("**#: Change Password");

          } else {
            lcd.clear();
            lcd.print("Incorrect Password!");
            delay(1000);
            displayLoginMenu();
          }

        } else {
          lcd.print(keypadPress);
          input += keypadPress;
        }
      }
    } else {
      if (lcdActive) {    // If LCD was on
        lcd.noDisplay();  // Turn off LCD display
        lcdActive = false;
        Serial.println("LCD Off");
        disableKeypad();
      }
    }
  }



  if (digitalRead(pirSensorPin) == HIGH) {
    Serial.println("Warning: Ada orang disekitar brankas!");
  }

  delay(1000);
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

void displayLoginMenu(){
  lcd.cursor();
  lcd.blink();
  lcd.clear();
  lcd.print("LOGIN MENU");
  lcd.setCursor(0, 1);
  lcd.print("Password: ");
}
void displayMainMenu(){
  
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
void shutDownSystem() {
  InfoComms.print("DC");
}

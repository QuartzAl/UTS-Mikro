/* BATTERY MODULE ARDUINO 3*/
#include <Wire.h>

int powerCutStatus = 0;
const int redLEDPin = 10;              // Pin untuk LED merah
const int redPin = 6;                  // Pin untuk komponen merah LED RGB
const int greenPin = 3;                // Pin untuk komponen hijau LED RGB
const int bluePin = 4;                 // Pin untuk komponen biru LED RGB
const int potentiometerPin = A0;       // Pin untuk membaca potensiometer
const int simulatorPower = 2;          // Pin untuk membaca saklar
const float fullBatteryVoltage = 5.0;  // Tegangan penuh baterai
const float minBatteryVoltage = 4.0;   // Tegangan minimum baterai untuk pengisian
bool approved = false;

void setup() {
  pinMode(2, INPUT_PULLUP);                                              // Konfigurasi pin sebagai input dengan internal pull-up resistor untuk sinyal interrupt
  attachInterrupt(digitalPinToInterrupt(2), powerCutInterrupt, CHANGE);  // Attach interrupt untuk mendeteksi perubahan pada pin-2
  Wire.begin(8);                                                         // Inisialisasi komunikasi I2C, dengan alamat I2C modul "Power" (misalnya, 8)
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);                                          // Mengatur fungsi yang akan dipanggil saat permintaan I2C diterima
  Serial.begin(9600);
  pinMode(10, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(simulatorPower, INPUT_PULLUP);
  powerCutInterrupt();
}

void loop() {
  delay(50);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
  //Serial.println(voltage);

  if (voltage < 4.0) {
    digitalWrite(6, HIGH);
    digitalWrite(10, HIGH);
    delay(500);
    digitalWrite(10, LOW);
    delay(500);
  } else if (voltage <= 4.5) {
    digitalWrite(3, HIGH);
    digitalWrite(10, HIGH);
    delay(1000);
    digitalWrite(10, LOW);
    delay(1000);
  } else {
    digitalWrite(4, HIGH);
  }
}
void receiveEvent(int bytes) {
  if (bytes == 3) {
    char received[4];
    int i = 0;
    while (Wire.available()) {
      received[i] = Wire.read();
      i++;
    }
    received[i] = '\0';  // Add null terminator to end of string

    if (strcmp(received, "PWR") == 0) {
      approved = true;
    }
  }
}

void requestEvent() {
  if (!approved) { return; }
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
  String voltageString = String(voltage, 1);
  Serial.println("Sent Voltage: " + voltageString + "V");
  
  if (powerCutStatus == 1){
    Wire.write(voltageString.c_str());
  }else{
    Wire.write("0.0");
  }
  approved = false;
}

void types(String a) {
  Serial.println("it's a String");
}
void types(int a) {
  Serial.println("it's an int");
}
void types(char *a) {
  Serial.println("it's a char*");
}
void types(float a) {
  Serial.println("it's a float");
}
void types(bool a) {
  Serial.println("it's a bool");
}


void powerCutInterrupt() {
  // Baca status dari simulator power-cut dan simpan di powerCutStatus
  powerCutStatus = digitalRead(2);
}

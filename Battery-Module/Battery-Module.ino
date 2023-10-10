#include <Wire.h>





const int redLEDPin = 10;  // Pin untuk LED merah
const int redPin = 6;      // Pin untuk komponen merah LED RGB
const int greenPin = 3;    // Pin untuk komponen hijau LED RGB
const int bluePin = 4;     // Pin untuk komponen biru LED RGB
const int potentiometerPin = A0; // Pin untuk membaca potensiometer
const int switchPin = 2;   // Pin untuk membaca saklar
const float fullBatteryVoltage = 5.0; // Tegangan penuh baterai
const float minBatteryVoltage = 4.0;  // Tegangan minimum baterai untuk pengisian

void setup() {
  Wire.begin(4);                // join i2c bus with address #4
  //Wire.onRequest(requestEvent); // register event
  Serial.begin(9600);
  pinMode(10, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop() {
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
  Serial.println(voltage);
  
  if(voltage < 4.0){
    digitalWrite(6, HIGH);
    digitalWrite(10, HIGH);
    delay(500);
    digitalWrite(10, LOW);
    delay(500);
  }
  else if(voltage <= 4.5){
    digitalWrite(3, HIGH);
    digitalWrite(10, HIGH);
    delay(1000);
    digitalWrite(10, LOW);
    delay(1000);
  }
  else{
    digitalWrite(4, HIGH);
  }
}

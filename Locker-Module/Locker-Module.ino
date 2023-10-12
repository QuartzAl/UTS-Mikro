/* LOCKER MODULE ARDUINO 2*/
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <Servo.h>

const byte temperature = A0;
const byte gasSensor = A2;
const byte alarm = 10;
const byte rxPin = 11;
const byte txPin = 12;
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3
int readingTemp;
int readingGas;
float temperatureVal;
bool allowed = true;
char charInput;
char keyInput;


//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
{'1', '2', '3', ' '},
{'4', '5', '6', ' '},
{'7', '8', '9', ' '},
{'*', '0', '#', ' '}
};

SoftwareSerial InfoComms (rxPin, txPin);
Servo myservo;

//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

void setup(){
  pinMode(temperature, INPUT);
  pinMode(gasSensor, INPUT);
  pinMode(alarm, OUTPUT);
  Serial.begin(9600);
  InfoComms.begin(4800);
  myservo.attach(13);
}

void loop(){
  delay(50);
  keyInput = myKeypad.getKey();
  
  if (allowed && keyInput != NO_KEY){
    Serial.println("key inputted: " + keyInput);
    InfoComms.print(keyInput);
  }
  
  readingTemp = analogRead(temperature);
  temperatureVal = (readingTemp*(5.0 / 1024.0) - 0.5)* 100;
  readingGas = analogRead(gasSensor);
  
  if (temperatureVal > 100 && readingGas > 200){
    closeVault();
    InfoComms.print('!');
  } else if(InfoComms.available() > 0) {
    charInput = InfoComms.read();
    if (charInput == 'C'){
      closeVault();
      Serial.println("Vault closed");
    }else if (charInput == 'O' ){
      openVault();
      Serial.println("Vault opened");
    }else if (charInput == 'E'){
      enableKeypad();
      Serial.println("keypad enabled");
    }else if (charInput == 'D'){
      disableKeypad();
      Serial.println("keypad disabled");
    }
  }
}

void closeVault(){
	myservo.write(0);
}
void openVault(){
	myservo.write(90);
}

void disableKeypad(){
  allowed = false;
}
void enableKeypad(){
  allowed = true;
}

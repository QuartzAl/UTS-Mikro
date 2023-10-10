

// Arduino Uno R3 ke-2
/* ********** Modul Locker ************

Modul Locker ini memiliki tugas:
1. Membaca input keypad dan mengirimkannya via Software Serial 
   ke modul Info. Catatan: untuk SoftwareSerial, gunakan 
   baudrate 4800
   Perhatikan bahwa modul "Locker" ini seharusnya TIDAK membaca
   keypad jika LCD dalam posisi mati (modul "Info" seharusnya
   mengirim status/kondisi ini ke modul "Locker")
   Modul "Locker" ini TIDAK PERLU mengirimkan data apapun
   via serial-USB ke komputer (tapi silahkan/boleh dipakai untuk
   proses debugging)
2. Mendeteksi kebakaran dengan sensor suhu DAN sensor asap.
   - Kebakaran dianggap terjadi jika KEDUA sensor tersebut aktif
   - Jika kebakaran terjadi:
     1. Nyalakan alarm/buzzer
     2. Servo dikembalikan pada posisi tertutup (jika saat itu
        dia terbuka)
     3. Kirimkan notifikasi ke modul "Info" bahwa telah terjadi
        kebakaran (dimana nantinya modul "Info" akan mem-forward
        informasi kebakaran ini ke pengguna/pemilik lewat 
        Serial-USB)
3. Membuka dan menutup servo sesuai perintah dari modul Info,
   KECUALI jika terjadi kebakaran (akan langsung/tetap menutup)
4. Ada kemungkinan modul "Info" akan mengirim perintah untuk
   shutdown sistem secara keseluruhan. Jika ini terjadi, maka
   modul "Locker" ini harus menutup servo (jika dalam kondisi 
   terbuka).

Arduino ini bertugas untuk membaca masukan dari keypad. 
Perhatikan fungsi-fungsi penting keypad berikut (yang akan 
diproses oleh modul Info):
   '#' -> tombol Enter (setelah memasukkan digit-digit password
   '*' -> jika ditekan pada saat sedang memasukkan password,
           maka digit-digit yang sudah dimasukkan akan dihapus
           (clear) dan akan diulang dari awal lagi
   '*#' -> menutup brankas (jika sudah berhasil login)
   '**#' -> untuk mereset password (harus sudah login)

*/

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
}

void loop(){
  delay(10);
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

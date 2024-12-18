#include <Arduino.h>
#include <SoftwareSerial.h>



// Define the SoftwareSerial pins
#define rxPin 18 // RX pin to Arduino (connect to TX of RN2483)
#define txPin 17 // TX pin from Arduino (connect to RX of RN2483)
#define rst 15

const uint8_t devEUI[8] = {0xD4, 0x55, 0x99, 0xA4, 0x39, 0x36, 0xC8, 0xB4};    // D45599A43936C8B4
const uint8_t appEUI[8] = {0x0D, 0xAE, 0xA4, 0x0E, 0x1D, 0xF3, 0xAA, 0x6E}; //0DAEA40E1DF3AA6E
const uint8_t appKey[16] = {0x20, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, }; //20240000000000000000000000000000


String rst_response = "";
int set_flag = 0;

// Create a SoftwareSerial object
SoftwareSerial loraSerial(rxPin, txPin);


// Timer pointer
hw_timer_t *My_timer = NULL;

// Function to send a command to RN2483 and print the response to Serial Monitor
void loraSendCommand(String command) {
  // Clear any previous data in the buffer
  while (loraSerial.available()) {
    loraSerial.read();
  }

  // Send the command to the RN2483 module
  loraSerial.print(command + "\r\n");

  // Wait for a response
  String response = "";
  unsigned long timeout = millis() + 5000; // 5 seconds timeout à changer ça peut buger
  while (millis() < timeout) {
    if (loraSerial.available()) {
      char c = loraSerial.read();
      response += c;
      // Check if response ends with "\r\n"
      if (response.endsWith("\r\n")) {
        break;
      }
    }
  }

  // Print the response to the Serial Monitor
  Serial.print("Command: " + command);
  Serial.println("   Response: " + response);
}



void MacSet(String command, const uint8_t * key, int size) {
  String hexKey;
  // Clear any previous data in the buffer
  while (loraSerial.available()) {
    loraSerial.read();
  }

  for (int i = 0; i<size; i++ ) {
    if (key[i] < 0x10) hexKey += "0"; // Add leading zero
    hexKey += String(key[i], HEX);
  }
  hexKey.toUpperCase(); // Ensure uppercase HEX

  // Send the command to the RN2483 module
  loraSerial.print("mac set " + command + " " + hexKey + "\r\n");

  // Wait for a response
  String response = "";
  unsigned long timeout = millis() + 5000; // 5 seconds timeout à changer ça peut buger
  while (millis() < timeout) {
    if (loraSerial.available()) {
      char c = loraSerial.read();
      response += c;
      // Check if response ends with "\r\n"
      if (response.endsWith("\r\n")) {
        break;
      }
    }
  }

  // Print the response to the Serial Monitor
  Serial.print("mac set " + command + " " + hexKey);
  Serial.println("   Response: " + response);
}



void loraSendData(String sensor, float value) {
  int id = 0;
  // Clear any previous data in the buffer
  while (loraSerial.available()) {
    loraSerial.read();
  }

  switch (sensor[0]) {
    case 'G' : //Gas sensor
      id = 0;
    case 'H' : //Humidity
      id = 1;
    case 'T' : //Temperature
      id = 2;
    default :
      id = 666; //no sensor /!/ belek
  }

  // Send the command to the RN2483 module
  loraSerial.print("mac tx cnf 1 " + id + String(value) + "\r\n"); // cnf = confirmed, 1 = port (between 1 and 256)

  // Wait for a response
  String response = "";
  unsigned long timeout = millis() + 5000; // 5 seconds timeout à changer ça peut buger
  while (millis() < timeout) {
    if (loraSerial.available()) {
      char c = loraSerial.read();
      response += c;
      // Check if response ends with "\r\n"
      if (response.endsWith("\r\n")) {
        break;
      }
    }
  }

  // Print the response to the Serial Monitor
  Serial.print("Command: mac tx cnf 1 " + id + String(value));
  Serial.println("   Response: " + response);
}


void setup() {
  // Initialize the Serial Monitor
  Serial.begin(115200);
  // Initialize the SoftwareSerial port
  loraSerial.begin(57600);

  delay(10000); //wait for the arduino ide's serial console to open

  Serial.println("Startup");

  // timer settings (4 available timers)
  /*
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 5000000, true);
  timerAlarmEnable(My_timer);
  */
}

void loop() {
  Serial.println("reset RN2483");
  pinMode(rst, OUTPUT);
  digitalWrite(rst, LOW);
  delay(100);
  digitalWrite(rst, HIGH);
  delay(10000);

  loraSendCommand("sys get vdd");


  //loraSendCommand("mac pause");
  //delay(1000);
  //loraSendCommand("mac set pwridx 1");
  //delay(1000);
  //MacSet("devuei", devEUI, 8);
  //delay(1000);
  loraSendCommand("sys get hweui");
  delay(1000);
  //MacSet("appeui", appEUI, 8); //don't tested yet, might work
  loraSendCommand("mac set appeui 0DAEA40E1DF3AA6E");
  delay(1000);
  MacSet("appkey", appKey, 16);
  delay(1000);
  loraSendCommand("mac save");
  delay(1000);
  //loraSendCommand("mac resume");
  //delay(1000);

  loraSendCommand("mac join otaa"); // Start OTAA join
  delay(1000);

  while(1){
    delay(1000);
  }
}

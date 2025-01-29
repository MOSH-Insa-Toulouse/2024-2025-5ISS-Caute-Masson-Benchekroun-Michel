#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin definitions for LoRa
#define LORA_RX_PIN 10
#define LORA_TX_PIN 11
SoftwareSerial loraSerial(LORA_RX_PIN, LORA_TX_PIN);

// Pin definitions for Bluetooth HC-05
#define BT_RX_PIN 2
#define BT_TX_PIN 3
SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN);

// Pin for gas sensor
#define GAS_SENSOR_PIN A0

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Start serial communication
  Serial.begin(9600);
  loraSerial.begin(57600);
  btSerial.begin(9600);
  delay(1000);

  // Initialisation de l'écran OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Initialisation..."));
  display.display();
  delay(2000);

  // Initialize LoRa
  Serial.println("Starting LoRa RN2483 configuration...");
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  delay(1000);
  digitalWrite(7, HIGH);
  waitForResponse();

  sendCommand("mac reset 868");
  sendCommand("mac set dr 5");
  sendCommand("mac set adr off");
  sendCommand("mac set ar off");
  sendCommand("mac set deveui 0004A30B002109EF");
  sendCommand("mac set appeui 3D75E50F79A51BE9");
  sendCommand("mac set appkey 3BB0E4729335CE0AF5F677CA7F320510");
  sendCommand("mac save");
  sendCommand("mac join otaa");
  waitForResponse();
}

void loop() {
  // Lecture de la valeur brute du capteur de gaz
  int gasValue = analogRead(GAS_SENSOR_PIN);

  // Conversion en tension (en supposant une alimentation de 5V)
  float voltage = gasValue * (5.0 / 1023.0);

  // Affichage sur l'écran OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Capteur de gaz"));
  display.print(F("Valeur brute: "));
  display.println(gasValue);
  display.print(F("Tension: "));
  display.print(voltage);
  display.println(F(" V"));
  display.display();

  // Send gas value over LoRa
  char hexValue[8];
  sprintf(hexValue, "%04X", gasValue); // Convert to hex
  String payload = "mac tx uncnf 1 " + String(hexValue);
  sendCommand(payload);

  delay(10000); // Wait 10 second for the regeneration of the sensor

  // Send gas value over Bluetooth
  btSerial.print("Gas Sensor Value: ");
  btSerial.println(gasValue);
  Serial.println("Data sent over Bluetooth");
}

void sendCommand(const String& command) {
  loraSerial.println(command);
  Serial.print("Sent: ");
  Serial.println(command);
  waitForResponse();
}

void waitForResponse() {
  while (!loraSerial.available()) {
    delay(10);
  }
  while (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(response);
  }
}









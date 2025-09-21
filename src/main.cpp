#include <Arduino.h>
#include <LED.h>
#include <Adafruit_MCP3008.h>
#include <pico/multicore.h>
#include <vector>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MCP3008Reader.h>
#include <PicoGamepad.h>
//#include <Oversample.h>
#include <EMA.h>

#define DEBUG

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//
// LED Class
// A simple class to control an LED connected to a specified pin
//
LED led(LED_BUILTIN, 1000); // Initialize LED on built-in pin with 100ms toggle interval

//
// Initialize MCP3008
//
const int8_t MCP3008_CS_PIN = 17;              // Chip Select pin for MCP3008
const uint8_t MCP3008_CHANNELS = 8;            // Number of channels to read
const uint8_t MCP3008_VALUES_PER_CHANNEL = 21; // Number of values to store per channel
//const uint16_t MCP3008_PROC_TICK_TIME = 10;    // Time interval for reading channels in milliseconds
Adafruit_MCP3008 adcChip;
MCP3008Reader adcMCP3008(&adcChip, MCP3008_CHANNELS, MCP3008_VALUES_PER_CHANNEL);

// Initialize I2C for EEPROM
arduino::MbedI2C Wire1(6, 7);
// Init EEPROM
I2C_eeprom eeprom(0x50, I2C_DEVICESIZE_24LC64, &Wire1); // I2C address for the EEPROM

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

// Initialize PicoGamepad
PicoGamepad joystick;

// Log to Serial
void logToSerial(const String &message)
{
  if (Serial)
  {
    Serial.println(String(__FILE__) + " --- " + message);
  }
}

// uitn16_t ertek ira EEPROM-ba a megadott cimen
int updateUint16ToEEPROM(int address, uint16_t value)
{
  uint8_t lowByte = value & 0xFF;         // Az alsó bájt
  uint8_t highByte = (value >> 8) & 0xFF; // A felső bájt

  int statusLow = eeprom.updateByte(address, lowByte);
  if (statusLow != 0)
  {
    return statusLow; // Hibakód visszaadása, ha az alsó bájt írása sikertelen
  }

  int statusHigh = eeprom.updateByte(address + 1, highByte);
  return statusHigh; // Visszatér a felső bájt írásának státuszával (0 ha sikeres)
}

// uint16_t ertek kiolvasasa az EEPROM-bol a megadott cimrol
uint16_t readUint16FromEEPROM(int address)
{
  uint8_t lowByte = eeprom.readByte(address);      // Az alsó bájt
  uint8_t highByte = eeprom.readByte(address + 1); // A felső bájt

  return (static_cast<uint16_t>(highByte) << 8) | lowByte; // Egyesítjük a két bájtot egy uint16_t értékké
}

int ch4_limter_min; // Minimum limit for channel 4
int ch4_limter_max; // Maximum limit for channel 4

//
// Setup function
// Initializes the MCP3008 and launches the core1 task
//
void setup()
{
  // LED off
  led.off();

  // Init Serial and wait for connection
  if (!Serial)
    Serial.begin(115200);
  logToSerial("Program started");

  // Init I2C for EEPROM
  logToSerial("I2C_EEPROM_VERSION: " + String(I2C_EEPROM_VERSION));

  Wire1.begin();  // Join I2C bus as master
  eeprom.begin(); // Initialize EEPROM
  if (!eeprom.isConnected())
  {
    logToSerial("EEPROM not connected!");
    while (true)
    {
      led.refresh(100);
    }
  }
  else
  {
    logToSerial("EEPROM connected.");
  }

  // Init OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    logToSerial("SSD1306 allocation failed");
    while (true)
    {
      led.refresh(100);
    }
  }
  else
  {
    logToSerial("SSD1306 connected.");
  }
  display.clearDisplay();
  display.display();

  // Read limits from EEPROM
  ch4_limter_min = readUint16FromEEPROM(0);
  ch4_limter_max = readUint16FromEEPROM(2);

  // Init MCP3008

  if (!adcChip.begin(MCP3008_CS_PIN))
  {
    // Hibajelzés: gyors villogás vagy végtelen ciklus
    while (1)
    {
      led.toggle();
      delay(100);
    }
  }
}

void loop()
{
  static uint32_t lastTime = 0;
  
  adcMCP3008.readChannelsWithEMA();

  if (millis() - lastTime > 50)
  {
    lastTime = millis();
    joystick.SetX(adcMCP3008.getMappedJoystickValue(CHANNEL_HAND_WHEEL));
    joystick.SetY(adcMCP3008.getMappedJoystickValue(CHANNEL_RUDDER));
    joystick.SetRx(adcMCP3008.getMappedJoystickValue(CHANNEL_THROTTLE_LEFT));
    joystick.SetRy(adcMCP3008.getMappedJoystickValue(CHANNEL_THROTTLE_RIGHT));
    joystick.SetSlider(adcMCP3008.getMappedJoystickValue(CHANNEL_BRAKE_LEFT));
    joystick.SetDial(adcMCP3008.getMappedJoystickValue(CHANNEL_BRAKE_RIGHT));
    joystick.send_update();
  }
}
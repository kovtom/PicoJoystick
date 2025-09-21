#ifndef MCP3008READER_H
#define MCP3008READER_H


#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include <algorithm> // sort, max_element
#include <Adafruit_MCP3008.h>
#include <EMA.h>

const int MAX_ADC_VALUE = 1023; // Maximum ADC value for MCP3008
const int CHANNEL_COUNT = 8; // Total number of channels

const int JOYSTICK_MIN_VALUE = -32767; // Minimum joystick value
const int JOYSTICK_MAX_VALUE = 32767;  // Maximum joystick value

const int CHANNEL_THROTTLE_LEFT = 0; // Channel number for left throttle
const int CHANNEL_THROTTLE_RIGHT = 1; // Channel number for right throttle
const int CHANNEL_RUDDER = 2; // Channel number for rudder
const int CHANNEL_BRAKE_LEFT = 3; // Channel number for brake
const int CHANNEL_BRAKE_RIGHT = 4; // Channel number for brake
const int CHANNEL_HAND_WHEEL = 5; // Channel number for hand wheel
const int CHANNEL_EMPTY_1 = 6; // Channel number not used
const int CHANNEL_EMPTY_2 = 7; // Channel number not used

struct channelMixMaxValues
{
    int minValue;
    int maxValue;
    bool isInverted = false; // Alapértelmezés szerint nem invertált
    bool isActive = true;  // Alapértelmezés szerint aktív
};

const channelMixMaxValues channelMinMaxValues_[] = {
    {360, 631, true, true}, // CHANNEL_THROTTLE_LEFT
    {273, 767, false, true},  // CHANNEL_THROTTLE_RIGHT
    {275, 783, false, true}, // CHANNEL_RUDDER
    {197, 715, false, true}, // CHANNEL_BRAKE_LEFT
    {176, 704, false, true}, // CHANNEL_BRAKE_RIGHT
    {299, 804, false, true}, // CHANNEL_HAND_WHEEL
    {0, 1023, false, false},  // CHANNEL_EMPTY_1
    {0, 1023, false, false}   // CHANNEL_EMPTY_2
};

//
// Osztaly vagy tipus mely ket adatot tartalmaz:
// - egy mert erteket 16 bites integer formaban
// - egy 8 bites interger mely a meres sorszamat tartalmazza
struct MeasuredValue {
  int16_t value;   // A mért érték
  uint8_t index;  // A mérés sorszáma
};

//
// Az MCP3008-rol származó analóg bemenetek olvasására szolgáló osztály
// Az Adafruit_MCP3008 osztálybol származik,
// mely tarolja a beolvasott MeasuredValue értékeket egy 2D vektorban
//
class MCP3008Reader {
public:
  MCP3008Reader(Adafruit_MCP3008* adc, const uint8_t channelNumber, const uint8_t arraySize, const uint16_t procTickTime);
  
  // Az aktuális csatornaértékek beolvasása és tárolása a values_ vektorban
  bool refresh();

  std::vector<std::vector<MeasuredValue>> values_;

  // Visszatér a megadott csatorna kozepso ertekevel
  int16_t getMedianValue(uint8_t channel);

  // Map-olt joystick adatok lekerese az adott csatornarol
  int16_t getMappedJoystickValue(uint8_t channel);

  // Az csatornak ertekinek beolvasasa es tarolasa az adcValue_buffer_ vektorban
  void readChannels();

  void readChannelsWithEMA();

  uint32_t getEMAValues(uint8_t channel);

private:
  uint8_t arraySize_;
  uint8_t CHANNEL_NUMBER_;
  std::vector<int16_t> adcValue_buffer_;
  uint16_t procTickTime_;
  uint32_t lastTickTime_;
  Adafruit_MCP3008* adc_;
  EMA<8, uint32_t> ema_[CHANNEL_COUNT]; // EMA szűrők minden csatornához
  uint32_t emaValues_[CHANNEL_COUNT] = {0}; // EMA értékek tárolása minden csatornához

  // Az adcValue_buffer_ ertekek beszurasa a values_ vektorba a legnagyobb indexu helyre
  void insertNewValues();

  // A vektorban levo index ertekek oregitese, mely soran az index ertekek eggyel novekszenek
  void ageIndices();

  // Az adott csatorna vektorat sorbarendezi az ertekek alapjan
  void sortChannelByValue(uint8_t channel);

  // Az osszes csatorna vektorának sorbarendezése az ertekek alapjan
  void sortAllChannelsByValue();
};
#endif // MCP3008READER_H
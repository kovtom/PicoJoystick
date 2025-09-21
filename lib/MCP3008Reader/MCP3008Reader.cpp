#include <MCP3008Reader.h>

MCP3008Reader::MCP3008Reader(Adafruit_MCP3008 *adc, const uint8_t channelNumber, const uint8_t arraySize, const uint16_t procTickTime = 1000)
    : adc_(adc)
{
    arraySize_ = arraySize;
    CHANNEL_NUMBER_ = channelNumber;
    procTickTime_ = procTickTime;
    lastTickTime_ = millis();
    values_.resize(CHANNEL_NUMBER_);
    for (uint8_t ch = 0; ch < CHANNEL_NUMBER_; ++ch)
    {
        values_[ch].resize(arraySize_);
        for (uint8_t i = 0; i < arraySize_; ++i)
        {
            values_[ch][i].value = 0;
            values_[ch][i].index = i;
        }
    }
    adcValue_buffer_.resize(CHANNEL_NUMBER_, 0);
}

bool MCP3008Reader::refresh()
{
    if (millis() - lastTickTime_ > procTickTime_)
    {
        lastTickTime_ = millis(); // Az utolsó frissítési idő frissítése

        ageIndices();      // Az indexek "öregítése"
        readChannels();    // Az aktuális csatornaértékek beolvasása
        insertNewValues(); // Az új értékek beszúrása a values_ vektorba
        // sortAllChannelsByValue(); // Az összes csatorna vektorának sorbarendezése
        return true;
    }
    return false;
}

int16_t MCP3008Reader::getMedianValue(uint8_t channel)
{
    if (channel >= CHANNEL_NUMBER_)
        return -1; // Hibakezeles: ervenytelen csatorna

    // A csatorna vektorának sorbarendezése az ertekek alapjan
    sortChannelByValue(channel);

    // A kozepso ertek visszateritese
    return values_[channel][arraySize_ / 2].value;
}

void MCP3008Reader::readChannels()
{
    for (uint8_t ch = 0; ch < CHANNEL_NUMBER_; ++ch)
    {
        adcValue_buffer_[ch] = adc_->readADC(ch);
    }
}

void MCP3008Reader::readChannelsWithEMA()
{
    for (uint8_t ch = 0; ch < CHANNEL_NUMBER_; ++ch)
    {
        emaValues_[ch] = ema_[ch](adc_->readADC(ch));
    }
}

uint32_t MCP3008Reader::getEMAValues(uint8_t channel) {
    return emaValues_[channel];
}


void MCP3008Reader::insertNewValues()
{
    for (uint8_t ch = 0; ch < CHANNEL_NUMBER_; ++ch)
    {
        // Megkeressuk a legnagyobb indexu helyet
        auto maxIt = std::max_element(values_[ch].begin(), values_[ch].end(),
                                      [](const MeasuredValue &a, const MeasuredValue &b)
                                      {
                                          return a.index < b.index;
                                      });
        if (maxIt != values_[ch].end())
        {
            maxIt->value = adcValue_buffer_[ch];
            maxIt->index = 0; // Az új érték indexe 0 lesz
        }
    }
}

void MCP3008Reader::ageIndices()
{
    for (auto &channelValues : values_)
    {
        for (auto &mv : channelValues)
        {
            mv.index++;
        }
    }
}

void MCP3008Reader::sortChannelByValue(uint8_t channel)
{
    if (channel >= CHANNEL_NUMBER_)
        return; // Hibakezeles: ervenytelen csatorna

    std::sort(values_[channel].begin(), values_[channel].end(),
              [](const MeasuredValue &a, const MeasuredValue &b)
              {
                  return a.value < b.value; // Növekvő sorrend
              });
}

void MCP3008Reader::sortAllChannelsByValue()
{
    for (uint8_t ch = 0; ch < CHANNEL_NUMBER_; ++ch)
    {
        sortChannelByValue(ch);
    }
}

int16_t MCP3008Reader::getMappedJoystickValue(uint8_t channel) {
    //int16_t rawValue = getMedianValue(channel);
    uint32_t rawValue = getEMAValues(channel);
    if (channel >= CHANNEL_NUMBER_ || !channelMinMaxValues_[channel].isActive) {
        return 0; // Hibakezelés: érvénytelen csatorna vagy inaktív csatorna esetén 0-t ad vissza
    }
    // Az ertek map-olasa a joystick ertekek tartomanyara
    if(rawValue < channelMinMaxValues_[channel].minValue) rawValue = channelMinMaxValues_[channel].minValue;
    if(rawValue > channelMinMaxValues_[channel].maxValue) rawValue = channelMinMaxValues_[channel].maxValue;
    if(channelMinMaxValues_[channel].isInverted) {
        // Invertált esetben fordítva map-oljuk
        return map(rawValue, channelMinMaxValues_[channel].maxValue, channelMinMaxValues_[channel].minValue, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
    }
    // Normál esetben simán map-oljuk
    return map(rawValue, channelMinMaxValues_[channel].minValue, channelMinMaxValues_[channel].maxValue, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
}

#include <MCP3008Reader.h>

MCP3008Reader::MCP3008Reader(Adafruit_MCP3008 *adc, 
                            const uint8_t channelNumber,
                            const uint8_t arraySize)
    : adc_(adc)
{
    arraySize_ = arraySize;
    CHANNEL_NUMBER_ = channelNumber;
}

// Az csatornak ertekinek beolvasasa es tarolasa az adcValue_buffer_ vektorban
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

// Map-olt joystick adatok lekerese az adott csatornarol
int16_t MCP3008Reader::getMappedJoystickValue(uint8_t channel) {
    //int16_t rawValue = getMedianValue(channel);
    uint32_t rawValue = getEMAValues(channel);
    if (channel >= CHANNEL_NUMBER_ || !channelMinMaxValues_[channel].isActive) {
        return 0; // Hibakezelés: érvénytelen csatorna vagy inaktív csatorna esetén 0-t ad vissza
    }
    // Az ertek map-olasa a joystick ertekek tartomanyara
    //if(rawValue < channelMinMaxValues_[channel].minValue) rawValue = channelMinMaxValues_[channel].minValue;
    //if(rawValue > channelMinMaxValues_[channel].maxValue) rawValue = channelMinMaxValues_[channel].maxValue;
    rawValue = constrain(rawValue, channelMinMaxValues_[channel].minValue, channelMinMaxValues_[channel].maxValue);
    if(channelMinMaxValues_[channel].isInverted) {
        // Invertált esetben fordítva map-oljuk
        return map(rawValue, channelMinMaxValues_[channel].maxValue, channelMinMaxValues_[channel].minValue, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
    }
    // Normál esetben simán map-oljuk
    return map(rawValue, channelMinMaxValues_[channel].minValue, channelMinMaxValues_[channel].maxValue, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
}

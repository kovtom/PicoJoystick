#include "Arduino.h"
#include <Adafruit_MCP3008.h>

#ifndef Oversample_h
#define Oversample_h

class Oversample
{
  public:
    /**
     * Total Resolution, including oversampled Bits.
     *
     * This may range from 10(no oversampling) to 16(max oversampling)
     * The more resolution you want, the longer the measurement will take.
     *
     * @param adc Pointer to an Adafruit_MCP3008 instance.
     * @param channel MCP3008 channel to use (0-7).
     * @param resolution Chosen resolution.
     */
    Oversample(Adafruit_MCP3008* adc, uint8_t channel, byte resolution);

    /**
     * Run a measurement with the set resolution.
     *
     * @return The oversampled, decimated measurement.
     */
    double read();

    /**
     * Run a measurement with the set resolution.
     *
     * @return The oversampled, non decimated measurement.
     */
    unsigned long readDecimated();

    /**
     * Set measurment resolution.
     *
     * @param resolution Resolution might be between _baseResolution and
     *                   _maxResolution.
     */
    void setResolution(byte resolution);
    byte getResolution();

  private:
    Adafruit_MCP3008* _adc;
    uint8_t _channel;
    byte _resolution;
    byte _baseResolution;
    byte _maxResolution;
    byte _additionalBits;
    int _sampleCount;

    byte sanitizeResolution(byte resolution);
};

#endif
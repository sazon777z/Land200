#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <DFRobotDFPlayerMini.h>
#include "config.h"

class AudioDriver {
public:
    AudioDriver();
    void begin();
    void playTrack(int trackNumber);
    void stop();
    void setVolume(uint8_t volume);
    void playAlarmSound();

private:
    DFRobotDFPlayerMini myDFPlayer;
    void printDetail(uint8_t type, int value);
};

#endif // AUDIO_DRIVER_H

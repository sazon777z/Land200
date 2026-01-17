#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include "config.h"
#include <DFRobotDFPlayerMini.h>


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
  uint8_t lastVolume = 255;
  void printDetail(uint8_t type, int value);
};

#endif // AUDIO_DRIVER_H

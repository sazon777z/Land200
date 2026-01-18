#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include "config.h"
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Типы команд для аудио-очереди
enum AudioCmdType { CMD_PLAY, CMD_STOP, CMD_SET_VOLUME, CMD_PLAY_ALARM };

struct AudioCommand {
  AudioCmdType type;
  int data;
};

class AudioDriver {
public:
  AudioDriver();
  void begin();
  void playTrack(int trackNumber);
  void stop();
  void setVolume(uint8_t volume);
  void playAlarmSound(int trackNumber);

  // Метод для обработки очереди (вызывается в отдельной задаче)
  void processQueue();

private:
  DFRobotDFPlayerMini myDFPlayer;
  uint8_t lastVolume = 255;
  QueueHandle_t audioQueue;
  bool isReady = false;
  void printDetail(uint8_t type, int value);

  // Внутренние блокирующие методы
  void executePlay(int track);
  void executeStop();
  void executeVolume(int vol);
};

#endif // AUDIO_DRIVER_H

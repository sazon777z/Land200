#include "AudioDriver.h"
#include <Arduino.h>
#include <HardwareSerial.h>

// Use HardwareSerial 1 for ESP32-C3
HardwareSerial mySerial(1);

AudioDriver::AudioDriver() {}

void AudioDriver::begin() {
  Serial.println("Audio: Initializing DFPlayer...");

  // Инициализация Serial
  mySerial.begin(9600, SERIAL_8N1, PIN_AUDIO_RX, PIN_AUDIO_TX);

  // Даем время на стабилизацию питания модуля
  delay(1500);

  // Используем (mySerial, false, false) - отключаем ACK и Reset
  // Это самый надежный способ для клонов, которые зависают при инициализации
  if (!myDFPlayer.begin(mySerial, false, false)) {
    Serial.println(
        "Audio Warning: DFPlayer begin returned false. Wiring/SD issue?");
    // Мы все равно разрешаем работу, так как команды могут уходить "вслепую"
  } else {
    Serial.println("Audio: DFPlayer initialized.");
  }

  isReady = true;
  myDFPlayer.volume(25);
  lastVolume = 25;

  // Создание очереди команд
  audioQueue = xQueueCreate(20, sizeof(AudioCommand));
}

void AudioDriver::playTrack(int trackNumber) {
  if (audioQueue == NULL)
    return;
  AudioCommand cmd = {CMD_PLAY, trackNumber};
  xQueueSend(audioQueue, &cmd, 0);
}

void AudioDriver::stop() {
  if (audioQueue == NULL)
    return;
  AudioCommand cmd = {CMD_STOP, 0};
  xQueueSend(audioQueue, &cmd, 0);
}

void AudioDriver::setVolume(uint8_t volume) {
  if (audioQueue == NULL)
    return;
  AudioCommand cmd = {CMD_SET_VOLUME, (int)volume};
  xQueueSend(audioQueue, &cmd, 0);
}

void AudioDriver::playAlarmSound() {
  if (audioQueue == NULL)
    return;
  AudioCommand cmd = {CMD_PLAY_ALARM, 0};
  xQueueSend(audioQueue, &cmd, 0);
}

void AudioDriver::processQueue() {
  if (audioQueue == NULL || !isReady)
    return;

  AudioCommand cmd;
  if (xQueueReceive(audioQueue, &cmd, pdMS_TO_TICKS(10)) == pdPASS) {
    Serial.printf("Audio: Command (Type: %d, Data: %d)\n", cmd.type, cmd.data);
    switch (cmd.type) {
    case CMD_PLAY:
      executePlay(cmd.data);
      break;
    case CMD_STOP:
      executeStop();
      break;
    case CMD_SET_VOLUME:
      executeVolume(cmd.data);
      break;
    case CMD_PLAY_ALARM:
      executePlay(1);
      break;
    }
  }
}

void AudioDriver::executePlay(int trackNumber) {
  if (trackNumber < 1)
    trackNumber = 1;
  if (trackNumber > 7)
    trackNumber = 7;

  Serial.printf("Audio: playFolder(1, %d)\n", trackNumber);
  myDFPlayer.playFolder(1, trackNumber);
  delay(150);
}

void AudioDriver::executeStop() {
  Serial.println("Audio: stop()");
  myDFPlayer.stop();
  delay(100);
}

void AudioDriver::executeVolume(int volume) {
  if (volume > 30)
    volume = 30;
  Serial.printf("Audio: volume(%d)\n", volume);
  myDFPlayer.volume(volume);
  lastVolume = volume;
  delay(100);
}

void AudioDriver::printDetail(uint8_t type, int value) {
  // Not used but kept for interface consistency
}

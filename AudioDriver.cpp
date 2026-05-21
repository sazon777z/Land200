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

  // Даем время на стабилизацию питания модуля (сокращено с 1500 до 500 мс)
  delay(500);

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

void AudioDriver::playAlarmSound(int trackNumber) {
  if (audioQueue == NULL)
    return;
  AudioCommand cmd = {CMD_PLAY_ALARM, trackNumber};
  xQueueSend(audioQueue, &cmd, 0);
}

void AudioDriver::playFolderTrack(int folder, int trackNumber) {
  if (audioQueue == NULL)
    return;
  AudioCommand cmd = {CMD_PLAY_FOLDER_TRACK, (folder << 16) | (trackNumber & 0xFFFF)};
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
      executePlay(cmd.data);
      break;
    case CMD_PLAY_FOLDER_TRACK:
      executePlayFolderTrack(cmd.data >> 16, cmd.data & 0xFFFF);
      break;
    }
  }
}

void AudioDriver::executePlay(int trackNumber) {
  if (trackNumber < 1)
    trackNumber = 1;
  if (trackNumber > 10)
    trackNumber = 10;

  Serial.printf("Audio: playFolder(1, %d)\n", trackNumber);
  myDFPlayer.playFolder(1, trackNumber);
  // Небольшая пауза для приема команды модулем (не блокирует основной поток —
  // вызов происходит уже в TaskNetwork, задача которого может отдать CPU)
  vTaskDelay(pdMS_TO_TICKS(150));
}

void AudioDriver::executePlayFolderTrack(int folder, int trackNumber) {
  if (folder < 1)
    folder = 1;
  if (trackNumber < 1)
    trackNumber = 1;

  Serial.printf("Audio: playFolder(%d, %d)\n", folder, trackNumber);
  myDFPlayer.playFolder(folder, trackNumber);
  vTaskDelay(pdMS_TO_TICKS(150));
}

void AudioDriver::executeStop() {
  Serial.println("Audio: stop()");
  myDFPlayer.stop();
  vTaskDelay(pdMS_TO_TICKS(50));
}

void AudioDriver::executeVolume(int volume) {
  if (volume > 30)
    volume = 30;
  Serial.printf("Audio: volume(%d)\n", volume);
  myDFPlayer.volume(volume);
  lastVolume = volume;
  // Пауза не нужна — модуль принимает команду без задержки подтверждения
}

void AudioDriver::printDetail(uint8_t type, int value) {
  // Not used but kept for interface consistency
}

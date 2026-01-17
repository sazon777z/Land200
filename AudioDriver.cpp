#include "AudioDriver.h"
#include <Arduino.h>
#include <HardwareSerial.h>


// Use HardwareSerial 1 for ESP32-C3 if available or remap 0
// For C3 SuperMini, explicit assignment is often best
HardwareSerial mySerial(1);

AudioDriver::AudioDriver() {}

void AudioDriver::begin() {
  // Initialize Serial for DFPlayer
  Serial.println("Audio: Initializing DFPlayer...");
  mySerial.begin(9600, SERIAL_8N1, PIN_AUDIO_RX, PIN_AUDIO_TX);

  // Give clones more time to wake up
  delay(1000);

  bool connected = false;
  for (int i = 0; i < 3; i++) {
    Serial.print("Audio: Attempt ");
    Serial.print(i + 1);
    Serial.println("...");
    // For MP3-TF-16P-V3 and similar clones:
    // Some require reset (true), some ACK (true).
    // Default is ACK=true, Reset=true. Let's try default but with better
    // logging.
    if (myDFPlayer.begin(mySerial)) {
      connected = true;
      break;
    }
    delay(500);
  }

  if (!connected) {
    Serial.println("Audio Error: DFPlayer NOT found! Check pins/card.");
  } else {
    Serial.println("Audio: DFPlayer Online.");
    myDFPlayer.volume(20);
    lastVolume = 20;
  }
}

void AudioDriver::playTrack(int trackNumber) {
  if (trackNumber < 1)
    trackNumber = 1;
  if (trackNumber > 7)
    trackNumber = 7;

  Serial.print("Audio: Playing track ");
  Serial.println(trackNumber);

  // Clear any pending serial data
  while (mySerial.available())
    mySerial.read();

  // Using playFolder(1, trackNumber) for folder "01" as requested
  myDFPlayer.playFolder(1, trackNumber);
  delay(150); // Give time for the command to be processed
}

void AudioDriver::stop() {
  myDFPlayer.stop();
  delay(100);
  myDFPlayer.stop(); // Send twice for reliability with clones
}

void AudioDriver::setVolume(uint8_t volume) {
  if (volume > 30)
    volume = 30;

  // Cache volume to prevent constant spamming of commands
  if (volume == lastVolume)
    return;
  lastVolume = volume;

  Serial.print("Audio: Set volume to ");
  Serial.println(volume);
  myDFPlayer.volume(volume);
  delay(100); // Wait for module to process volume change
}

void AudioDriver::playAlarmSound() {
  playTrack(1); // Use the stabilized playTrack method
}

#include "AudioDriver.h"
#include <HardwareSerial.h>

// Use HardwareSerial 1 for ESP32-C3 if available or remap 0
// For C3 SuperMini, explicit assignment is often best
HardwareSerial mySerial(1);

AudioDriver::AudioDriver() {
}

void AudioDriver::begin() {
    // Initialize Serial for DFPlayer
    mySerial.begin(9600, SERIAL_8N1, PIN_AUDIO_RX, PIN_AUDIO_TX);
    
    if (!myDFPlayer.begin(mySerial)) {
        // Handle error: Serial.println(F("Unable to begin:"));
        // Serial.println(F("1.Please recheck the connection!"));
        // Serial.println(F("2.Please insert the SD card!"));
    }
    
    myDFPlayer.volume(15);  // Set volume value. From 0 to 30
}

void AudioDriver::playTrack(int trackNumber) {
    myDFPlayer.play(trackNumber);
}

void AudioDriver::stop() {
    myDFPlayer.stop();
}

void AudioDriver::setVolume(uint8_t volume) {
    myDFPlayer.volume(volume);
}

void AudioDriver::playAlarmSound() {
    // Play track 1 as default alarm
    myDFPlayer.play(1);
}

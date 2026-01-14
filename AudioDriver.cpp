#include "AudioDriver.h"
#include <HardwareSerial.h>

// Use HardwareSerial 1 for ESP32-C3 if available or remap 0
// For C3 SuperMini, explicit assignment is often best
HardwareSerial mySerial(1);

AudioDriver::AudioDriver() {
}

void AudioDriver::begin() {
    // Initialize Serial for DFPlayer
    Serial.println("Audio: Initializing DFPlayer...");
    mySerial.begin(9600, SERIAL_8N1, PIN_AUDIO_RX, PIN_AUDIO_TX);
    
    // Give clones more time to wake up
    delay(1000); 
    
    bool connected = false;
    for (int i = 0; i < 3; i++) {
        Serial.print("Audio: Attempt "); Serial.print(i + 1); Serial.println("...");
        // isACK = false, doReset = true (default)
        // Disabling ACK helps with many clones that work but don't respond fast enough
        if (myDFPlayer.begin(mySerial, false)) {
            connected = true;
            break;
        }
        delay(500);
    }
    
    if (!connected) {
        Serial.println("Audio Warning: DFPlayer reported error, but might still work.");
    } else {
        Serial.println("Audio: DFPlayer Online.");
        myDFPlayer.volume(20); 
    }
}

void AudioDriver::playTrack(int trackNumber) {
    // Clear any pending serial data to ensure command is received
    while(mySerial.available()) mySerial.read();
    myDFPlayer.play(trackNumber);
}

void AudioDriver::stop() {
    myDFPlayer.stop();
    delay(100); 
    myDFPlayer.stop(); // Send twice for reliability with clones
}

void AudioDriver::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    myDFPlayer.volume(volume);
}

void AudioDriver::playAlarmSound() {
    while(mySerial.available()) mySerial.read();
    myDFPlayer.play(1);
}

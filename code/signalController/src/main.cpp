#include <Arduino.h>

/*
 * A simple single freq AD9850 Arduino test script
 * Original AD9851 DDS sketch by Andrew Smallbone at www.rocketnumbernine.com
 * Modified for testing the inexpensive AD9850 ebay DDS modules
 * Pictures and pinouts at nr8o.dhlpilotcentral.com
 * 9850 datasheet at http://www.analog.com/static/imported-files/data_sheets/AD9850.pdf
 * Use freely
 */

#include <Encoder.h>
#include <DDS.h>

#define DDS1_W_CLK 6       // connect to AD9850 module word load clock pin (CLK)
#define DDS1_FQ_UD 7       // connect to freq update pin (FQ)
#define DDS1_DATA 8       // connect to serial data load pin (DATA) - shared
#define DDS1_RESET 9      // connect to reset pin (RST) - shared

#define DDS2_W_CLK 10       // connect to AD9850 module word load clock pin (CLK)
#define DDS2_FQ_UD 11      // connect to freq update pin (FQ)
#define DDS2_DATA 8       // connect to serial data load pin (DATA) - shared
#define DDS2_RESET 9      // connect to reset pin (RST) - shared

#define ENCODER_MAX_STEPS 600

Encoder encoder1(2, 3);
Encoder encoder2(4, 5);
long enc1_oldPosition = 0;
long enc2_oldPosition = 0;

DDS dds1(DDS1_W_CLK, DDS1_FQ_UD, DDS1_DATA, DDS1_RESET);
DDS dds2(DDS2_W_CLK, DDS2_FQ_UD, DDS2_DATA, DDS2_RESET);

String serialBuffer = "";         // a String to hold incoming data
boolean newSerialMessage = false;

// exponential growing function
double calculateFrequency(long enc1_pos, long enc2_pos) {
  double mainFreq = exp((float)enc1_pos / ENCODER_MAX_STEPS * 6.8);
  double fineFreq = mainFreq * 0.1 * enc2_pos / ENCODER_MAX_STEPS;
  return mainFreq + fineFreq;
}

void clearSerialMessage() {
  newSerialMessage = false;
  serialBuffer = "";
}

bool comparePrefix(String string, String prefix) {
  if (string.length() < prefix.length())
    return false;
  for (int i=0;i<prefix.length();i++) {
    if (string[i] != prefix[i])
      return false;
  }
  return true;
}

void setup() {
  dds1.init();
  dds2.init();

  Serial.begin(115200);

  enc1_oldPosition = 0;
  enc2_oldPosition = 0;

  encoder1.write(enc1_oldPosition);
  encoder2.write(enc2_oldPosition);

  double startFreq = calculateFrequency(enc1_oldPosition, enc2_oldPosition);

  dds1.setFrequency(startFreq);
  dds2.setFrequency(startFreq*2);

  serialBuffer.reserve(100);
  Serial.println("started");
}

void loop() {

  // read serial messages
  if (newSerialMessage) {
    if (comparePrefix(serialBuffer, "f1:")) {
      double frequency = serialBuffer.substring(3).toDouble();
      Serial.print(serialBuffer);
      dds1.setFrequency(frequency);

    } else if (comparePrefix(serialBuffer, "f2:")) {
      double frequency = serialBuffer.substring(3).toDouble();
      Serial.print(serialBuffer);
      dds2.setFrequency(frequency);
    }
    clearSerialMessage();
  }

  // read encoder positions
  long enc1_newPosition = encoder1.read();
  long enc2_newPosition = encoder2.read();
  if (enc1_newPosition != enc1_oldPosition || enc2_newPosition != enc2_oldPosition) {

    enc1_oldPosition = constrain(enc1_newPosition, 0, ENCODER_MAX_STEPS);
    encoder1.write(enc1_oldPosition);

    enc2_oldPosition = enc2_newPosition;

    double frequency = calculateFrequency(enc1_newPosition, enc2_newPosition);

    dds1.setFrequency(frequency);
    dds2.setFrequency(frequency*2);

    Serial.print("f1:");
    Serial.println(frequency);

    Serial.print("f2:");
    Serial.println(frequency * 2);
  }
}

void serialEvent() {
  while (Serial.available() && !newSerialMessage) {
    char inChar = (char)Serial.read();
    serialBuffer += inChar;
    if (inChar == '\n') {
      newSerialMessage = true;
    }
  }
}

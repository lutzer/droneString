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

#define W_CLK 8       // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9       // Pin 9 - connect to freq update pin (FQ)
#define DATA 10       // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11      // Pin 11 - connect to reset pin (RST).

#define ENCODER_MAX_STEPS 600

Encoder encoder1(3, 4);
Encoder encoder2(5, 6);
long enc1_oldPosition = 0;
long enc2_oldPosition = 0;

DDS dds(W_CLK, FQ_UD, DATA, RESET);

// exponential growing function
double calculateFrequency(long enc1_pos, long enc2_pos) {
  double mainFreq = exp((float)enc1_pos / ENCODER_MAX_STEPS * 6.8);
  double fineFreq = mainFreq * 0.1 * enc2_pos / ENCODER_MAX_STEPS;
  return mainFreq + fineFreq;
}

void setup() {
  dds.init();

  Serial.begin(9600);

  enc1_oldPosition = ENCODER_MAX_STEPS * 0.9;
  enc2_oldPosition = 0;

  encoder1.write(enc1_oldPosition);
  encoder2.write(enc2_oldPosition);

  dds.setFrequency(calculateFrequency(enc1_oldPosition, enc2_oldPosition));

  Serial.println("started");
}

void loop() {
  long enc1_newPosition = encoder1.read();
  long enc2_newPosition = encoder2.read();
  if (enc1_newPosition != enc1_oldPosition || enc2_newPosition != enc2_oldPosition) {

    enc1_oldPosition = constrain(enc1_newPosition, 0, ENCODER_MAX_STEPS);
    encoder1.write(enc1_oldPosition);

    enc2_oldPosition = enc2_newPosition;

    double frequency = calculateFrequency(enc1_newPosition, enc2_newPosition);

    dds.setFrequency(frequency);
    
    Serial.print("f1:");
    Serial.println(frequency);
  }
}

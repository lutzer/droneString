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
 //#include <AD9850.h>

 #define W_CLK 8       // Pin 8 - connect to AD9850 module word load clock pin (CLK)
 #define FQ_UD 9       // Pin 9 - connect to freq update pin (FQ)
 #define DATA 10       // Pin 10 - connect to serial data load pin (DATA)
 #define RESET 11      // Pin 11 - connect to reset pin (RST).

 #define ENCODER_MAX_STEPS 600

 #define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }

Encoder encoder1(3, 4);
Encoder encoder2(5, 6);
long enc1_oldPosition = 0;
long enc2_oldPosition = 0;

 // transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data)
{
  for (int i=0; i<8; i++, data>>=1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

 // frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) {
  int32_t freq = frequency * 4294967295/125000000;  // note 125 MHz clock on 9850
  for (int b=0; b<4; b++, freq>>=8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}

void setup() {
 // configure arduino data pins for output
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // this pulse enables serial mode - Datasheet page 12 figure 10

  Serial.begin(9600);

  encoder1.write(ENCODER_MAX_STEPS * 0.9);
  encoder2.write(0);

  Serial.println("started");
}

void loop() {
  long enc1_newPosition = encoder1.read();
  long enc2_newPosition = encoder2.read();
  if (enc1_newPosition != enc1_oldPosition || enc2_newPosition != enc2_oldPosition) {
    enc1_oldPosition = constrain(enc1_newPosition, 0, ENCODER_MAX_STEPS);
    encoder1.write(enc1_oldPosition);
    // if (enc1_newPosition <= 0) {
    //   enc1_oldPosition = 0;
    //   encoder2.write(0);
    //   encoder1.write(enc1_oldPosition);
    // } else if (enc1_newPosition > ENCODER_MAX_STEPS) {
    //   enc1_oldPosition = ENCODER_MAX_STEPS;
    //   encoder2.write(0);
    // }
    enc2_oldPosition = enc2_newPosition;
      // range from e^0 to e^8.0 (3000 hz), so make that the maximum
    double mainFreq = exp((float)enc1_newPosition / ENCODER_MAX_STEPS * 6.8);
    double fineFreq = mainFreq * 0.1 * enc2_newPosition / ENCODER_MAX_STEPS;
    double frequency =  mainFreq + fineFreq;
    Serial.println(frequency);
    sendFrequency(frequency);
  }
    // freq
}

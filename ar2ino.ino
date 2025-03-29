#if defined ARDUINO_ARCH_RENESAS_UNO
#include "FspTimer.h"
#include "encoderR4.h"
#else
#include "Encoder.h"
#endif

#define RLEN 4
#define WLEN 6
#define MAXEVENT 7

unsigned char BuffIn[RLEN];
unsigned char BuffOut[WLEN];

#if defined ARDUINO_ARCH_RENESAS_UNO
EncoderR4 enc(4, 5);
int32_t encCounter = 0;
#else
Encoder enc(2, 3);
#endif

int event = -1;

void setup() {
#if defined ARDUINO_ARCH_RENESAS_UNO
  analogWriteResolution(12);
#endif
  Serial.begin(230400);

#if defined ARDUINO_ARCH_RENESAS_UNO
  auto ret = enc.begin();
#endif

  Serial.setTimeout(50000);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

static void Init() {
  int type = BuffIn[2];
  switch (type) {
    case 0:
      // INPUT
      pinMode(BuffIn[1], INPUT);
      break;
    case 1:
      pinMode(BuffIn[1], OUTPUT);
      break;
    case 2:
#if defined ARDUINO_ARCH_RENESAS_UNO
      // TODO
#else
      enc.write(0);
#endif
      break;
  }
}

static void aRead() {
  BuffOut[0] = BuffIn[0];
  BuffOut[1] = BuffIn[1];
  int32_t *value = (int32_t *)&(BuffOut[2]);
  *value = analogRead(BuffIn[1]);

  Serial.write(BuffOut, WLEN);
}

static void pwmWrite() {
  int16_t *value = (int16_t *)&BuffIn[2];
  analogWrite(BuffIn[1], *value);
}

static void encRead() {
  BuffOut[0] = BuffIn[0];
  BuffOut[1] = BuffIn[1];
  int32_t *value = (int32_t *)&(BuffOut[2]);

#if defined ARDUINO_ARCH_RENESAS_UNO
  encCounter += enc.readChange();
  *value = encCounter;
#else
  *value = enc.read();
#endif

  Serial.write(BuffOut, WLEN);
}

static void dRead() {
  digitalWrite(13, HIGH);
  BuffOut[0] = BuffIn[0];
  BuffOut[1] = BuffIn[1];
  int32_t *value = (int32_t *)&(BuffOut[2]);

  Serial.write(BuffOut, WLEN);
}

static void dWrite() {
  int16_t *value = (int16_t *)&BuffIn[2];
  digitalWrite(BuffIn[1], *value);
}

static void aWrite() {
  int16_t *value = (int16_t *)&BuffIn[2];
  analogWrite(BuffIn[1], *value);
}

void (*const eventTable[MAXEVENT])(void) = { Init, aRead, pwmWrite, encRead, dRead, dWrite, aWrite };

void loop() {
  // put your main code here, to run repeatedly:
  int val;
  int ev;

  //  val = Serial.readBytes(BuffIn, RLEN);
  val = Serial.readBytes(BuffIn, RLEN);
  if (val == RLEN) {
    ev = BuffIn[0];
    if (ev < MAXEVENT) eventTable[ev]();
  }
}

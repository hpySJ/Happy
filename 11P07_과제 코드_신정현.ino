#include <Servo.h>

#define PIN_LED   9
#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define _DIST_MIN 180.0
#define _DIST_MAX 360.0
#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)
#define _EMA_ALPHA 0.3

#define _DUTY_MIN 0
#define _DUTY_MAX 2500

float dist_raw, dist_filtered, dist_prev = _DIST_MAX, dist_ema;
unsigned long last_sampling_time;
Servo myservo;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds((_DUTY_MIN + _DUTY_MAX) / 2);
  Serial.begin(57600);
}

void loop() {
  if (micros() < last_sampling_time + (INTERVAL * 1000UL)) return;

  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX) || (dist_raw < _DIST_MIN)) {
    dist_filtered = dist_prev;
    digitalWrite(PIN_LED, LOW);
  } else {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
    digitalWrite(PIN_LED, HIGH);
  }

  dist_ema = _EMA_ALPHA * dist_filtered + (1 - _EMA_ALPHA) * dist_prev;

  float duty;
  if (dist_ema <= _DIST_MIN) duty = _DUTY_MIN;
  else if (dist_ema >= _DIST_MAX) duty = _DUTY_MAX;
  else duty = map(dist_ema, _DIST_MIN, _DIST_MAX, _DUTY_MIN, _DUTY_MAX);

  myservo.writeMicroseconds((int)duty);

  Serial.print("dist_raw:"); Serial.print(dist_raw);
  Serial.print(", dist_ema:"); Serial.print(dist_ema);
  Serial.print(", duty:"); Serial.println(duty);

  last_sampling_time = micros();
}

float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}

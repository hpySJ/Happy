// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 100       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  
  Serial.begin(57600);
}

void loop() { 
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  float distance = USS_measure(PIN_TRIG, PIN_ECHO);

  // 거리 보정 (센서 범위 초과 시 처리)
  if ((distance == 0.0) || (distance > _DIST_MAX)) {
      distance = _DIST_MAX;    
  } else if (distance < _DIST_MIN) {
      distance = _DIST_MIN;    
  }

  // LED 밝기 계산 (active low)
  int brightness = 255;  // 기본값 = OFF
  if (distance <= _DIST_MIN || distance >= _DIST_MAX) {
    brightness = 255;  // 최소 (꺼짐)
  } else if (distance == 150 || distance == 250) {
    brightness = 128;  // duty 50%
  } else if (distance == 200) {
    brightness = 0;    // 최대 (가장 밝음)
  } else {
    // 100~200 구간: 255 → 0 선형 보간
    if (distance > 100 && distance < 200) {
      brightness = map((int)distance, 100, 200, 255, 0);
    }
    // 200~300 구간: 0 → 255 선형 보간
    else if (distance > 200 && distance < 300) {
      brightness = map((int)distance, 200, 300, 0, 255);
    }
  }

  analogWrite(PIN_LED, brightness);

  // Serial debug 출력
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" mm, Brightness: ");
  Serial.println(brightness);

  last_sampling_time = millis() ;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}

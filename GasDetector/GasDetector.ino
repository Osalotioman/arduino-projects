/*
 Gas detector program
 */
#include <SoftwareSerial.h>
#define SENSOR_A0 A0
#define SENSOR_D0 A1
#define ALARM_PIN 6

SoftwareSerial gsm(4, 5); // RX, TX

String phone_number = "+2348072114764";

unsigned long alarm_timer, sms_timer, logging_timer;
unsigned long sms_period = 90000, alarm_period = 0, logging_period = 1000;
bool alarm_state = true; // OFF inverted logic



void setup() {
  alarm_timer = sms_timer = logging_timer = millis();

  pinMode(SENSOR_A0, INPUT);
  pinMode(SENSOR_D0, INPUT);
  pinMode(ALARM_PIN, OUTPUT);
  digitalWrite(ALARM_PIN, LOW);

  Serial.begin(9600);
  gsm.begin(9600);
  gsm.println("AT");
  
  Serial.println("GAS DETECTOR DEVICE");
}

void updateSerial() {
  delay(500);
  while (gsm.available()) {
    Serial.write(gsm.read());
  }
  while (Serial.available()) {
    gsm.write(Serial.read());
  }
}

void sendCMD(String cmd) {
  gsm.println(cmd);
  updateSerial();
}



void loop() { // run over and over
  updateSerial();

  long intensity = constrain(map(analogRead(SENSOR_A0), 100, 700, 0, 100), 0, 100);
  bool gas_detected = !digitalRead(SENSOR_D0);
  alarm_period = map(intensity, 0, 100, 700, 100);

  if (millis() - logging_timer > logging_period) {
    Serial.println("Intensity: " + String(intensity));
    Serial.println("Gas detected: " + String(gas_detected));
    
    logging_timer = millis();
  }

  if (gas_detected) {
    sendCMD("AT");
    sendCMD("AT+CMGF=1");
    sendCMD("AT+CMGS=\"" + phone_number + "\"");
    sendCMD("EMERGENCY!!!!. GAS LEAK DETECTED!! ATTEND TO EMERGENCY");
    gsm.write(26);
    sendCMD("");

    long time = 7500;
    while (time > 0) {
      intensity = constrain(map(analogRead(SENSOR_A0), 100, 700, 0, 100), 0, 100);
      gas_detected = !digitalRead(SENSOR_D0);
      alarm_period = map(intensity, 0, 100, 700, 100);

      alarm_state = !alarm_state;
      digitalWrite(ALARM_PIN, alarm_state);
      time -= alarm_period;
      delay(alarm_period);
    }
    
  } else {
    digitalWrite(ALARM_PIN, HIGH);
  }

  if (intensity > 80) {
    sendCMD("AT");
    sendCMD("ATD+ " + phone_number + ";\r");
    long time = 90000;
    bool not_closed = true;
    while (time > 0) {
      intensity = constrain(map(analogRead(SENSOR_A0), 100, 700, 0, 100), 0, 100);
      gas_detected = !digitalRead(SENSOR_D0);
      alarm_period = map(intensity, 0, 100, 700, 100);

      alarm_state = !alarm_state;
      digitalWrite(ALARM_PIN, alarm_state);
      time -= alarm_period;
      delay(alarm_period);
      if (time < 70000 && not_closed) {
        gsm.println("ATH");
        not_closed = false;
      }
    }
  } 
}

#include <Servo.h>

#define servoPin 25
Servo myservo;

void setup() {
  myservo.attach(servoPin);
}

void loop() {
  myservo.write(0);     // move to 0 degrees
  delay(1000);          // wait 1 second

  myservo.write(90);    // move to 90 degrees (middle position)
  delay(1000);          // wait 1 second

  myservo.write(180);   // move to 180 degrees (max position)
  delay(1000);          // wait 1 second
}

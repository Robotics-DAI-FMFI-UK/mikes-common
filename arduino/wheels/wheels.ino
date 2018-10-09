// program for arduino that controls wheels - servo is connected to PIN D10
#include <Servo.h>

Servo s;
Servo d;

void setup()
{
  s.attach(10);
  d.attach(9);
  Serial.begin(115200);
  s.write(0);
  d.write(100);
}

void loop() 
{
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == 'M') Serial.write('M');
    else if (c == 'U') s.write(0);
    else if (c == 'D') s.write(180);
    else if (c == 'F') s.write(90);
    else if (c == 'T') test();
    else if (c == 'O') d.write(180);
    else if (c == 'C') d.write(100);
    else Serial.write('?');
  }
}

void test() 
{
  s.write(90);
  delay(1000);
  s.write(0);
  delay(1000);
  s.write(180);
  delay(1000);
}



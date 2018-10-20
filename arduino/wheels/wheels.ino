// program for arduino that controls wheels - servo is connected to PIN D10
#include <Servo.h>

#define TRIG 4
#define ECHO 3

Servo s;
Servo d;

void setup()
{
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  s.attach(10);
  d.attach(9);
  Serial.begin(115200);
  s.write(0);
  d.write(100);
}

int meraj()
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long tm = millis();

  while (digitalRead(ECHO) == 0)
  {
     //cakame na zaciatok pulzu
     if (millis() - tm > 20) break;
  }

  if (digitalRead(ECHO) == 0) return 200;

  long zaciatok = micros();
  while (digitalRead(ECHO) == 1)
  {
    //cakame na koniec pulzu
  }
  long koniec = micros();
  int vzdialenost = (koniec - zaciatok) / 58;
  return vzdialenost;
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
    else if (c == 'V') { if (meraj() < 20) Serial.write('P'); else Serial.write('V'); }
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



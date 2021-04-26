int trigs[] = { 17, 15, 5, 3, 13, 7, 9, 11 };
int echos[] = { 16, 14, 4, 2, 12, 6, 8, 10 };
int m[8];

/* indexes from 0 to 7:
LEFT                  
TOP_LEFT              
TOP_RIGHT             
RIGHT                 
MIDDLE_LEFT           
MIDDLE_RIGHT          
DOWN_LEFT             
DOWN_RIGHT            
*/

void setup() 
{
  Serial.begin(115200);
  for (int i = 0; i < 8; i++)
  {
    pinMode(echos[i], INPUT);
    pinMode(trigs[i], OUTPUT);
  }
}

int measure(int i) 
{
  int trig = trigs[i];
  int echo = echos[i];
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  unsigned long counter = 0;
  while (digitalRead(echo) == 0) { counter++; if (counter > 4000) return -1; }  
  long tm1 = micros();
  counter = 0;
  while (digitalRead(echo) == 1) { counter++; if (counter > 4000) return -1; }
  long tm2 = micros();
  return (tm2 - tm1) / 58;
}

void roundtrip()
{
  long t0 = millis();
  for (int i = 0; i < 8; i++)
  {
    while (millis() < t0 + 20 * i);
    m[i] = measure(i);
  }
}

void print_all()
{
  Serial.print("us ");
  for (int i = 0; i < 8; i++)
  {
    Serial.print(m[i]);
    if (i < 7) Serial.print(" ");
  }
  Serial.println();
}

void loop() 
{
  roundtrip();
  print_all();
}

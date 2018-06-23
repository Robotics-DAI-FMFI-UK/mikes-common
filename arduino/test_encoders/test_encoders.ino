// just prints status of encoders

void setup() 
{
  pinMode(A3, INPUT);  // encA1
  pinMode(3, INPUT);  // encA2
  pinMode(4, INPUT);  // encB1
  pinMode(8, INPUT);  // encB2
  pinMode(9, OUTPUT); // motorA
  pinMode(10, OUTPUT);// motorB

  pinMode(2, INPUT);  // interrupt pin for compass
  Serial.begin(115200);
}

void loop() 
{
  Serial.print(digitalRead(A3));
  Serial.print(" ");
  Serial.print(digitalRead(3));
  Serial.print(" ");
  Serial.print(digitalRead(4));
  Serial.print(" ");
  Serial.print(digitalRead(8));
  Serial.println();
  delay(200);
}

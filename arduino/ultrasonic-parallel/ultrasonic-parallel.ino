int trigs[] = { 17, 15, 5, 3, 13, 7, 9, 11 };
int echos[] = { 16, 14, 4, 2, 12, 6, 8, 10 };

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


/* TRIGS:
 
PC3, PC1, PD5, PD3, PB5, PD7, PB1, PB3

PB: 1, 3, 5
PC: 1, 3
PD: 3, 4, 7

ECHOS:

PC2, PC0, PD4, PD2, PB4, PD6, PB0, PB2
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

int16_t dist[8];
uint8_t how_many_responded = 0;
uint8_t state0, state1, state2, state3, state4, state5, state6, state7;
uint32_t tm0s, tm0e, tm1s, tm1e, tm2s, tm2e, tm3s, tm3e, tm4s, tm4e, tm5s, tm5e, tm6s, tm6e, tm7s, tm7e;

void  measure() 
{


  state0 = state1 = state2 = state3 = state4 = state5 = state6 = state7 = 0;
  
  PORTB |= 0b00101010;
  PORTC |= 0b00001010;
  PORTD |= 0b10101000;
  
  delayMicroseconds(10);

  PORTB &= 0b11010101;
  PORTC &= 0b11110101;
  PORTD &= 0b01010111;

  how_many_responded = 0;  
  unsigned long time_measurement_started = millis();
  
  while ((how_many_responded < 8) && (millis() - time_measurement_started < 20))
  {
    if (!state0)
    {
       if (PINC & 0b100)
       {
          state0 = 1;
          tm0s = micros();
       }
    }
    else if (state0 == 1)
    {
      if ((PINC & 0b100) == 0)
      {
        tm0e = micros();
        state0 = 2;
        how_many_responded++;
      }
    }
    if (!state1)
    {
       if (PINC & 0b1)
       {
          state1 = 1;
          tm1s = micros();
       }
    }
    else if (state1 == 1)
    {
      if ((PINC & 0b1) == 0)
      {
        tm1e = micros();
        state1 = 2;
        how_many_responded++;
      }
    }
    if (!state2)
    {
       if (PIND & 0b10000)
       {
          state2 = 1;
          tm2s = micros();
       }
    }
    else if (state2 == 1)
    {
      if ((PIND & 0b10000) == 0)
      {
        tm2e = micros();
        state2 = 2;
        how_many_responded++;
      }
    }
    if (!state3)
    {
       if (PIND & 0b100)
       {
          state3 = 1;
          tm3s = micros();
       }
    }
    else if (state3 == 1)
    {
      if ((PIND & 0b100) == 0)
      {
        tm3e = micros();
        state3 = 2;
        how_many_responded++;
      }
    }
    if (!state4)
    {
       if (PINB & 0b10000)
       {
          state4 = 1;
          tm4s = micros();
       }
    }
    else if (state4 == 1)
    {
      if ((PINB & 0b10000) == 0)
      {
        tm4e = micros();
        state4 = 2;
        how_many_responded++;
      }
    }
    if (!state5)
    {
       if (PIND & 0b1000000)
       {
          state5 = 1;
          tm5s = micros();
       }
    }
    else if (state5 == 1)
    {
      if ((PIND & 0b1000000) == 0)
      {
        tm5e = micros();
        state5 = 2;
        how_many_responded++;
      }
    }
    if (!state6)
    {
       if (PINB & 0b1)
       {
          state6 = 1;
          tm6s = micros();
       }
    }
    else if (state6 == 1)
    {
      if ((PINB & 0b1) == 0)
      {
        tm6e = micros();
        state6 = 2;
        how_many_responded++;
      }
    }
    if (!state7)
    {
       if (PINB & 0b100)
       {
          state7 = 1;
          tm7s = micros();
       }
    }
    else if (state7 == 1)
    {
      if ((PINB & 0b100) == 0)
      {
        tm7e = micros();
        state7 = 2;
        how_many_responded++;
      }
    }
    
  }

  if (state0 == 2)  dist[0] = (tm0e - tm0s) / 58;
  else dist[0] = -1;
  if (state1 == 2)  dist[1] = (tm1e - tm1s) / 58;
  else dist[1] = -1;
  if (state2 == 2)  dist[2] = (tm2e - tm2s) / 58;
  else dist[2] = -1;
  if (state3 == 2)  dist[3] = (tm3e - tm3s) / 58;
  else dist[3] = -1;
  if (state4 == 2)  dist[4] = (tm4e - tm4s) / 58;
  else dist[4] = -1;
  if (state5 == 2)  dist[5] = (tm5e - tm5s) / 58;
  else dist[5] = -1;
  if (state6 == 2)  dist[6] = (tm6e - tm6s) / 58;
  else dist[6] = -1;
  if (state7 == 2)  dist[7] = (tm7e - tm7s) / 58;
  else dist[7] = -1;
}

static long t0 = millis();

void roundtrip()
{ 
  while (millis() < t0 + 200);
  t0 = millis();
  measure();
}

void print_all()
{
  Serial.print("us ");
  for (int i = 0; i < 8; i++)
  {
    Serial.print(dist[i]);
    if (i < 7) Serial.print(" ");
  }
  Serial.println();
}

void loop() 
{
  roundtrip();
  print_all();
}

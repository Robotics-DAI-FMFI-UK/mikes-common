/*
  Calibrate HMC5883L + MPU6050 (GY-86 / GY-87). Output for HMC5883L_calibrate_processing.pde
  Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-magnetometr-hmc5883l.html
  GIT: https://github.com/jarzebski/Arduino-HMC5883L
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

/*

FTLAB:

1. -386.00,-309.00: -567,0: -326,20
1: -283,-62
2. -374.00,175.00: -567,0: -328,197: -283,-65

Waldkirch:

-277.00,-209.00: -509,87: -237,511: -211,137
-276.00,-208.00: -509,87: -237,511: -211,137

208.00,237.00: -415,75: -230,256: -170,13
-208.00,237.00: -415,75: -230,256: -170,13


Oct/7 compas 47cm height:
3. -331.00,-204.00: -438,182: -269,510: -128,120
4. 100.00,42.00: -449,154: -275,267: -147,-4


*/

#include <Wire.h>
#include "HMC5883L.h"
#include "MPU6050.h"

HMC5883L compass;
MPU6050 mpu;

int minX = 0;
int maxX = 0;
int minY = 0;
int maxY = 0;
int offX = 0;
int offY = 0;

void setup()
{
  Serial.begin(9600);

  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    delay(500);
  }

  // Enable bypass mode
  mpu.setI2CMasterModeEnabled(false);
  mpu.setI2CBypassEnabled(true);
  mpu.setSleepEnabled(false);

  // Initialize Initialize HMC5883L
  while (!compass.begin())
  {
    delay(500);
  }

  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  compass.setMeasurementMode(HMC5883L_CONTINOUS);

  // Set data rate
  compass.setDataRate(HMC5883L_DATARATE_30HZ);

  // Set number of samples averaged
  compass.setSamples(HMC5883L_SAMPLES_8);
}

long counter = 0;

void loop()
{
  Vector mag = compass.readRaw();

  // Determine Min / Max values
  if (mag.XAxis < minX) minX = mag.XAxis;
  if (mag.XAxis > maxX) maxX = mag.XAxis;
  if (mag.YAxis < minY) minY = mag.YAxis;
  if (mag.YAxis > maxY) maxY = mag.YAxis;

  // Calculate offsets
  offX = (maxX + minX)/2;
  offY = (maxY + minY)/2;

  // -374.00:261.00:-534:0:-149:287:-267:69
  //-324.00:259.00:-510:0:-146:327:-255:90
  //341.00:256.00:-516:0:-141:273:-258:66
  //-370.00,305.00: -543,0: -186,319: -271,66

  counter++;
  if (counter % 100 == 0)
  {  
    Serial.print(mag.XAxis);
    Serial.print(",");
    Serial.print(mag.YAxis);
    Serial.print(": ");
    Serial.print(minX);
    Serial.print(",");
    Serial.print(maxX);
    Serial.print(": ");
    Serial.print(minY);
    Serial.print(",");
    Serial.print(maxY);
    Serial.print(": ");
    Serial.print(offX);
    Serial.print(",");
    Serial.print(offY);
    Serial.println();
  }
}

#include "SCDVK.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Constants.h"

SCDVK DVK;

void setup()  
{
  DVK.begin();
}

void loop() // run over and over
{
  DVK.execute();
}


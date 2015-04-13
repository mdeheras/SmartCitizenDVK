#include "Constants.h"
#include "SCDVK.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define debug true

int firmware_mode = 0;
int operation_mode = 0;

unsigned long timeisr1 = 0;

void ISR1()
  {
   if ((millis()-timeisr1)>1000)
     {
       timeisr1 = millis();
       if (operation_mode<1)operation_mode++;
       else operation_mode=0;
     }
  }

unsigned long timeisr2 = 0; 

void ISR2()
  {
   if ((millis()-timeisr2)>1000)
     {
       timeisr2 = millis();
       if ((operation_mode>1)&&(operation_mode<3)) operation_mode++;
       else operation_mode=2;
     }
  }
  
void SCDVK::begin() {
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW); //RESET ON
  delay(100);
  digitalWrite(RESET, HIGH); //RESET OFF
  pinMode(MUX, OUTPUT);
  digitalWrite(MUX, LOW); //NORMAL MODE
  pinMode(PROG_MODE, OUTPUT);
  digitalWrite(PROG_MODE, LOW); //PROG_MODE OFF
  pinMode(GREEN, OUTPUT);
  digitalWrite(GREEN, HIGH); //GREEN ON
  pinMode(BLUE, OUTPUT);
  digitalWrite(BLUE, LOW); //BLUE OFF
  pinMode(MMC_CS, OUTPUT);
  pinMode(52, OUTPUT);
  digitalWrite(MMC_CS, LOW); //SPI MMC NO SELECT
  pinMode(RTX_CS, OUTPUT);
  digitalWrite(RTX_CS, LOW); //SPI RTX NO SELECT
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH); //ENABLE POWER RTX
  pinMode(SELECT_MODE_1, INPUT);
  pinMode(SELECT_MODE_2, INPUT);
  Serial.begin(9600);
  Serial1.begin(9600); 
  Wire.begin();
  attachInterrupt(SELECT_MODE_1, ISR1, FALLING) ;
  attachInterrupt(SELECT_MODE_2, ISR2, FALLING) ;
}

  
void SCDVK::disable_all()
  {
    Serial1.end();
    digitalWrite(MUX, HIGH); //PROGRAM MODE
    digitalWrite(SCK, LOW); //BLUE OFF
    digitalWrite(MISO, LOW); //BLUE OFF
    digitalWrite(MOSI, LOW); //BLUE OFF
  }

void SCDVK::enable_all() //Provisional function
  {
    Serial.begin(115200);
    Serial1.begin(115200); 
  }

void SCDVK::bootloader_flash()
  {
    disable_all();
    digitalWrite(RESET, LOW); //RESET OFF
    digitalWrite(GREEN, HIGH); //GREEN LED ON
    digitalWrite(BLUE, HIGH); //BLUE LED ON
    digitalWrite(POWER, LOW); //DISABLE POWER RTX
    delay(100);
    digitalWrite(PROG_MODE, HIGH);
    digitalWrite(RESET, HIGH); //RESET OFF
    delay(100);
    digitalWrite(POWER, HIGH);//ENABLE POWER RTX
    enable_all();
    delay(4000);
    digitalWrite(PROG_MODE, LOW);
    delay(100);
    digitalWrite(GREEN, LOW); //GREEN LED ON
    while (operation_mode==3) 
      {
        blink_led_blue(500);
        echo();
      }
  }

void SCDVK::firmware_flash()
  {
     digitalWrite(RESET, LOW); //RESET
     delay(100);
     digitalWrite(RESET, HIGH); //RESET OFF
     digitalWrite(MUX, HIGH); //PROGRAM MODE
     Serial.begin(115200);
     Serial1.begin(115200); 
     digitalWrite(GREEN, LOW); //GREEN LED OFF
     digitalWrite(BLUE, HIGH); //BLUE LED ON 
     delay(500);
     while (operation_mode==2) 
      {
        echo();
      }
  } 
  
void SCDVK::terminal_mode()
  {
    digitalWrite(MUX, LOW); //NORMAL MODE
    Serial.begin(9600);
    Serial1.begin(9600); 
    digitalWrite(GREEN, HIGH); //GREEN LED ON 
    digitalWrite(BLUE, LOW); //BLUE LED OFF 
    delay(500);
    while (operation_mode==1) 
      {
        blink_led_green(500);
        echo();
      }
  }

void SCDVK::normal_mode()
  {
    Serial.begin(9600);
    Serial1.begin(9600); 
    digitalWrite(GREEN, HIGH); //GREEN LED ON
    digitalWrite(BLUE, LOW); //GREEN LED ON
    while (operation_mode==0)
      {
        main();
      }
  }
  
boolean ledStateblue = LOW;
unsigned long previousMillisblue = 0; 

void SCDVK::blink_led_blue(unsigned long interval)
  {
    unsigned long currentMillisblue = millis();
    
    if(currentMillisblue - previousMillisblue >= interval) {
      // save the last time you blinked the LED 
      previousMillisblue = currentMillisblue;   
  
      // if the LED is off turn it on and vice-versa:
      if (ledStateblue == LOW)
        ledStateblue = HIGH;
      else
        ledStateblue = LOW;
  
      // set the LED with the ledState of the variable:
      digitalWrite(BLUE, ledStateblue);
    }
  }

boolean ledStategreen = LOW;
unsigned long previousMillisgreen = 0; 

void SCDVK::blink_led_green(unsigned long interval)
  {
    unsigned long currentMillisgreen = millis();
    
    if(currentMillisgreen - previousMillisgreen >= interval) {
      // save the last time you blinked the LED 
      previousMillisgreen = currentMillisgreen;   
  
      // if the LED is off turn it on and vice-versa:
      if (ledStategreen == LOW)
        ledStategreen = HIGH;
      else
        ledStategreen = LOW;
  
      // set the LED with the ledState of the variable:
      digitalWrite(GREEN, ledStategreen);
    }
  }
  
uint16_t SCDVK::readSHT21(uint8_t type){
      uint16_t DATA = 0;
      Wire.beginTransmission(SHT21_ADDRESS);
      Wire.write(type);
      Wire.endTransmission();
      Wire.requestFrom(SHT21_ADDRESS,2);
      unsigned long time = millis();
      while (!Wire.available()) if ((millis() - time)>500) return 0x00;
      DATA = Wire.read()<<8; 
      while (!Wire.available()); 
      DATA = (DATA|Wire.read()); 
      DATA &= ~0x0003; 
      return DATA;
  }

uint32_t lastHumidity;
uint32_t lastTemperature;

float SCDVK::getTemperature()
 {
  return  (-46.85 + ((175.72 * (float)readSHT21(0xE3)) / 65536.0) );
 }

float SCDVK::getHumidity()
 {
   return  (-6.0 + ((125.0  * (float)readSHT21(0xE5)) / 65536.0));
 }
  
// set up variables using the SD utility library functions:
Sd2Card card;

void SCDVK::check_MMC()
  {
      // we'll use the initialization code from the utility libraries
      // since we're just testing if the card is working!
      if (!card.init(SPI_QUARTER_SPEED, MMC_CS)) {
        Serial.println("initialization failed. Things to check:");
        Serial.println("* is a card is inserted?");
        Serial.println("* Is your wiring correct?");
        Serial.println("* did you change the chipSelect pin to match your shield or module?");
        return;
      } else {
        Serial.println("Wiring is correct and a card is present.");
      }
  }

void SCDVK::echo()
  {
      if (Serial.available())
          Serial1.write(Serial.read());
      if (Serial1.available())
        Serial.write(Serial1.read());
  }
  
void SCDVK::execute()
  {
      switch (operation_mode) {
      case 0:
        normal_mode();
        break;
      case 1:
        terminal_mode();
        break;
      case 2:
        firmware_flash();
        break;
      case 3:
        bootloader_flash();
        break;
    }
 }
 
 void SCDVK::main()
  {
    Serial.print("Temperature: ");
    Serial.print(getTemperature());
    Serial.print(" C, Humidity: ");
    Serial.print(getHumidity());
    Serial.println(" %");  
    delay(500);
    check_MMC();
  }

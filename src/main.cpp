/*
  main.cpp - Main entry to {PETPROJ22}
  Part of PETPROJ22
  
  Copyright (c) 2021 Aleksandr D. Kazakov | https://github.com/AlexanderDKazakov/
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#include <tools.h>

#define KEYBOARD_PIN         A0  // ANALOG
#define SIGNAL_PIN           A5  // ANALOG
#define MAX485_DE            3   // DIGITAL
#define MAX485_RE_NEG        2   // DIGITAL
#define ROTOR_VOLTAGE_PIN    6   // DIGITAL
#define WATER_PUMP_ON_LED    10  // DIGITAL
#define WATER_PUMP_COILS_LED 11  // DIGITAL
#define ROTOR_ON_LED         12  // DIGITAL

/*KEYBOARD*/
int     adc_key_in;
uint8_t NUM_KEYS           = 5;
int     key                = -1;
int     oldkey             = -1;

bool IS_WATER_PUMP_IN_USE  = true; // WATER_PUMP or ROTOR CHOICES
bool IS_ROTOR_IN_USE       = false;

/* ROTOR SETTINGS */
uint8_t ROTOR_SPEED        = 0;     // SPEED: 0
bool    ROTOR_STOPPED      = true;
int16_t ROTOR_SPEED_LOW    = 150;
int16_t ROTOR_SPEED_MID    = 200;
int16_t ROTOR_SPEED_HIG    = 255;

/*WATER PUMP DEFAULT: ON and STANDBY */
bool    WATER_PUMP         = true;  // READY: ON
uint8_t WATER_PUMP_SPEED   = 0;     // SPEED: 0
bool    WATER_PUMP_STOPPED = true;
int16_t WATER_PUMP_SPEED_LOW = 1600;
int16_t WATER_PUMP_SPEED_MID = 1800;
int16_t WATER_PUMP_SPEED_HIG = 3000;

/*WATER SENSOR -- the sensor works unstable ... */
bool IS_WSENSOR_IN_USE     = false;
int water_value            = 0;     // variable to store the sensor value

uint8_t result;

// ModbusMaster
ModbusMaster node;


void preTransmission()
{
  DigitalWrite(MAX485_RE_NEG, 1);
  DigitalWrite(MAX485_DE,     1);
}

void postTransmission()
{
  DigitalWrite(MAX485_RE_NEG, 0);
  DigitalWrite(MAX485_DE,     0);
}

// Convert ADC value to key number
int get_key(unsigned int input)
{
  if (  (input >= 0) && (input < 10) )  return 0;  // left   red
  if (  (input > 30) && (input < 50) )  return 1;  // right  red
  if (  (input > 85) && (input < 95) )  return 2;  // left   blue
  if ( (input > 125) && (input < 135) ) return 3;  // middle blue
  if ( (input > 535) && (input < 555) ) return 4;  // right  blue
  return -1;                                       // invalid input
}


void key_pressed(int key)
{
  if (key != oldkey)                       // if keypress is detected
  {
    delay(50);                             // wait for debounce time
    adc_key_in = analogRead(KEYBOARD_PIN); // read the value from the sensor

    key = get_key(adc_key_in);             // convert into key press
    if (key != oldkey)
    {
      oldkey = key;
      if (key >= 0)
      {
        DigitalWrite(13, HIGH);
        switch (key)
        {
        case 0:
          WATER_PUMP = !WATER_PUMP;
          if (WATER_PUMP) 
          {
            DigitalWrite(WATER_PUMP_COILS_LED, HIGH);
            node.writeSingleRegister(0x0003, 1);
          }
          else
          {
            DigitalWrite(WATER_PUMP_COILS_LED, LOW);
            node.writeSingleRegister(0x0003, 0);
          }
          Serial.println("WATER PUMP STATUS:");           // DEBUG
          Serial.println(WATER_PUMP);                     // DEBUG
          Serial.println("");                             // DEBUG
          break;

        case 1: // RED RIGHT
          /* WATER_PUMP and ROTOR swapping */
          
          if (IS_WATER_PUMP_IN_USE) 
          // ROTOR IN USE
          {
            IS_WATER_PUMP_IN_USE = false;
            IS_ROTOR_IN_USE      = true;

            Serial.println("ROTOR IN USE");
            DigitalWrite(WATER_PUMP_ON_LED, LOW);
            DigitalWrite(ROTOR_ON_LED, HIGH);
          }
          else 
          // WATER_PUMP IN USE
          {
            IS_WATER_PUMP_IN_USE = true;
            IS_ROTOR_IN_USE      = false;
            
            Serial.println("WATER PUMP IN USE");
            DigitalWrite(WATER_PUMP_ON_LED, HIGH);
            DigitalWrite(ROTOR_ON_LED, LOW);
          }
          break;

        case 2: // LEFT
          if (IS_WATER_PUMP_IN_USE)
          {
            
            if (WATER_PUMP_SPEED > 1) WATER_PUMP_SPEED--;

            if (WATER_PUMP_SPEED == 1) {
              result = node.writeSingleRegister(16385, WATER_PUMP_SPEED_LOW);
              if (result == node.ku8MBSuccess) Serial.println("WATER PUMP SPEED [SUCCESS]");
              else                             Serial.println("WATER PUMP SPEED [FAIL]");
              TIME_LAG;
            }

            if (WATER_PUMP_SPEED == 2) {
              result = node.writeSingleRegister(16385, WATER_PUMP_SPEED_MID);
              if (result == node.ku8MBSuccess) Serial.println("WATER PUMP SPEED [SUCCESS]");
              else                             Serial.println("WATER PUMP SPEED [FAIL]");
              TIME_LAG;
            }

            if (WATER_PUMP_SPEED == 3) {
              result = node.writeSingleRegister(16385, WATER_PUMP_SPEED_HIG);
              if (result == node.ku8MBSuccess) Serial.println("WATER PUMP SPEED [SUCCESS]");
              else                             Serial.println("WATER PUMP SPEED [FAIL]");
              TIME_LAG;
            }
          }

          if (IS_ROTOR_IN_USE)
          // ROTOR
          {
            if (ROTOR_SPEED > 1) ROTOR_SPEED--;

            if (ROTOR_SPEED == 1) {
              Serial.println("ROTOR SPEED 1");
              analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_LOW);
              TIME_LAG;
            }

            if (ROTOR_SPEED == 2) {
              Serial.println("ROTOR SPEED 2");
              analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_MID);
              TIME_LAG;
            }

            if (ROTOR_SPEED == 3) {
              Serial.println("ROTOR SPEED 3");
              analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_HIG);
              TIME_LAG;
            }

          }
          break;

        case 3: // STOP 
          if (IS_WATER_PUMP_IN_USE)
          {
            WATER_PUMP_STOPPED = true;
            WATER_PUMP_SPEED   = 0;
            result             = node.writeSingleRegister(0x0005, 0);
            if (result == node.ku8MBSuccess) Serial.println("WATER PUMP FINAL STOP [SUCCESS]");
            else                             Serial.println("WATER PUMP FINAL STOP [FAIL]");
            TIME_LAG;
          }
                    
          if ( (IS_ROTOR_IN_USE) && (WATER_PUMP_STOPPED) )
          // ROTOR
          {
            Serial.println("ROTOR SPEED 0");
            ROTOR_STOPPED = true;
            ROTOR_SPEED   = 0;
            analogWrite(ROTOR_VOLTAGE_PIN, 0);
            TIME_LAG;
          } 
          break;

        case 4: // RIGHT
          if (IS_WATER_PUMP_IN_USE)
          {
            if (WATER_PUMP_SPEED < 3) WATER_PUMP_SPEED++;

            if (WATER_PUMP_STOPPED) 
              {
                // TURN ON THE ROTOR AND WATER_SENSOR
                if (ROTOR_SPEED == 0)
                {
                  ROTOR_SPEED = 1;
                  analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_LOW);
                  TIME_LAG;
                }
                WATER_PUMP_STOPPED = false;
                result = node.writeSingleRegister(0x0005, 1);
                if (result == node.ku8MBSuccess) Serial.println("WATER PUMP FINAL MOVE [SUCCESS]");
                else                             Serial.println("WATER PUMP FINAL MOVE [FAIL]");
                TIME_LAG2X;
              }

            if (WATER_PUMP_SPEED == 1) {
                       
              result = node.writeSingleRegister(16385, WATER_PUMP_SPEED_LOW);
              if (result == node.ku8MBSuccess) Serial.println("WATER PUMP FINAL MOVE [SUCCESS]");
              else                             Serial.println("WATER PUMP FINAL MOVE [FAIL]");
              TIME_LAG;
            }

            if (WATER_PUMP_SPEED == 2) {
              result = node.writeSingleRegister(16385, WATER_PUMP_SPEED_MID);
              if (result == node.ku8MBSuccess) Serial.println("WATER PUMP SPEED [SUCCESS]");
              else                             Serial.println("WATER PUMP SPEED [FAIL]");
              TIME_LAG;
            }

            if (WATER_PUMP_SPEED == 3) {
              result = node.writeSingleRegister(16385, WATER_PUMP_SPEED_HIG);
              if (result == node.ku8MBSuccess) Serial.println("WATER PUMP SPEED [SUCCESS]");
              else                             Serial.println("WATER PUMP SPEED [FAIL]");
              TIME_LAG;
            }
          }
          
          if (IS_ROTOR_IN_USE)
          // ROTOR
          {
            if (ROTOR_SPEED < 3) ROTOR_SPEED++;

            if (ROTOR_SPEED == 1) {
              Serial.println("ROTOR SPEED 1");
              analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_LOW);
              TIME_LAG;
            }

            if (ROTOR_SPEED == 2) {
              Serial.println("ROTOR SPEED 2");
              analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_MID);
              TIME_LAG;
            }

            if (ROTOR_SPEED == 3) {
              Serial.println("ROTOR SPEED 3");
              analogWrite(ROTOR_VOLTAGE_PIN, ROTOR_SPEED_HIG);
              TIME_LAG;
            }
          }
          break;
        }
      }
    }
  }
}

int main(void)
{
  PinMode(13,                   OUTPUT);                  // DEBUG heartbeat
  PinMode(WATER_PUMP_COILS_LED, OUTPUT);                  // COILS:      AUTO LED ON
  PinMode(WATER_PUMP_ON_LED,    OUTPUT);                  // WATER_PUMP: AUTO LED ON
  PinMode(ROTOR_ON_LED,         OUTPUT);                  // ROTOR:      AUTO LED OFF
  
  if (IS_WATER_PUMP_IN_USE) 
  {
    DigitalWrite(WATER_PUMP_ON_LED, HIGH);
    DigitalWrite(ROTOR_ON_LED, LOW);
  }
  
  if (IS_ROTOR_IN_USE)
  {
    DigitalWrite(WATER_PUMP_ON_LED, LOW);
    DigitalWrite(ROTOR_ON_LED, HIGH);
  }
  if (WATER_PUMP) DigitalWrite(WATER_PUMP_COILS_LED, HIGH);

  // RS485
  PinMode(MAX485_RE_NEG, OUTPUT);
  PinMode(MAX485_DE,     OUTPUT);
  
  // Receive mode
  DigitalWrite(MAX485_RE_NEG, 0);
  DigitalWrite(MAX485_DE,     0);
  TIME_LAG;

  Serial.begin(9600);

  // Modbus slave ID 1
  node.begin(1, Serial);
  
  // Callbacks for RS425
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // SPEED
  result = node.writeSingleRegister(16385, 800);
  if (result == node.ku8MBSuccess) Serial.println("WATER PUMP SPEED [SUCCESS]");
  else                             Serial.println("WATER PUMP SPEED [FAIL]");
  TIME_LAG;

  // ACCELERATION
  result = node.writeSingleRegister(16387, 200);
  if (result == node.ku8MBSuccess) Serial.println("WATER PUMP ACCELERATION [SUCCESS]");
  else                             Serial.println("WATER PUMP ACCELERATION [FAIL]");
  TIME_LAG;

  // FINAL SPEED
  result = node.writeSingleRegister(16388, 1000);
  if (result == node.ku8MBSuccess) Serial.println("WATER PUMP FINAL SPEED [SUCCESS]");
  else                             Serial.println("WATER PUMP FINAL SPEED [FAIL]");
  TIME_LAG;

  // STARTING SPEED
  result = node.writeSingleRegister(16386, 400);
  if (result == node.ku8MBSuccess) Serial.println("WATER PUMP STARTING SPEED [SUCCESS]");
  else                             Serial.println("WATER PUMP STARTING SPEED [FAIL]");
  TIME_LAG;
  
  // CURRENT
  result = node.writeSingleRegister(16389, 900);
  if (result == node.ku8MBSuccess) Serial.println("WATER PUMP CURRENT [SUCCESS]");
  else                             Serial.println("WATER PUMP CURRENT [FAIL]");
  TIME_LAG;
    
  result = node.writeSingleRegister(0x0003, 1);
  if (result == node.ku8MBSuccess) Serial.println("WATER PUMP ON [SUCCESS]");
  else                             Serial.println("WATER PUMP ON [FAIL]");
  TIME_LAG;


  while (1)
  {
    DigitalWrite(13, LOW);
    water_value = analogRead(SIGNAL_PIN); // read the analog value from sensor
    
    // WATER IS COMING AND VOLTAGE IS PRESENT ON SENSOR AND WATER IS MORE THAN THRESHOLD --> turn off the water pump
    if ( (WATER_PUMP_STOPPED == false) && (ROTOR_SPEED > 0) && (water_value > 550) )
    {
      WATER_PUMP_STOPPED = true;
      WATER_PUMP_SPEED   = 0;
      result             = node.writeSingleRegister(0x0005, 0);
      if (result == node.ku8MBSuccess) Serial.println("WATER PUMP FINAL STOP [SUCCESS]");
      else                             Serial.println("WATER PUMP FINAL STOP [FAIL]");
      TIME_LAG;
    }

    adc_key_in = analogRead(KEYBOARD_PIN); // read the value from the sensor
    key        = get_key(adc_key_in);      // convert into key press
    key_pressed(key);
  }
}

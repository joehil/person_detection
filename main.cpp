#include <Arduino.h>
#include <Wire.h>
#include <RCSwitch.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "SparkFun_AK975X_Arduino_Library.h" //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_AK975X_Arduino_Library

#define RF_DATA 9
#define RF_VCC 8

AK975X movementSensor; //Hook object to the library
RCSwitch mySwitch = RCSwitch();

unsigned long address = 1342177280;

int ir1, ir2, ir3, ir4, irsum;

volatile unsigned int count = 0;

ISR(WDT_vect)
  /* Watchdog Timer Interrupt Service Routine */
  {
    count++;
  }

void setup()
{
  ADCSRA = ADCSRA & B01111111; // ADC abschalten, ADEN bit7 zu 0
  ACSR = B10000000; // Analogen Comparator abschalten, ACD bit7 zu 1
  DIDR0 = DIDR0 | B00111111; // Digitale Eingangspuffer ausschalten, analoge Eingangs Pins 0-5 auf 1

  /* Setup des Watchdog Timers */
  MCUSR &= ~(1<<WDRF);             /* WDT reset flag loeschen */
  WDTCSR |= (1<<WDCE) | (1<<WDE);  /* WDCE setzen, Zugriff auf Presclaler etc. */
  WDTCSR = 1<<WDP0 | 1<<WDP3;      /* Prescaler auf 8.0 s */
  WDTCSR |= 1<<WDIE;               /* WDT Interrupt freigeben */

    // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(RF_DATA);
  
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);

  // Optional set pulse length.
  mySwitch.setPulseLength(320);
  
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(3);

  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(RF_VCC, OUTPUT); 
  pinMode(RF_DATA, OUTPUT); 
  
  digitalWrite(RF_VCC, LOW);

  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT);

  pinMode(13, OUTPUT);
  digitalWrite(13,HIGH);

  Serial.begin(9600);
  Serial.println("AK975X Read Example");

  Wire.begin();

  //Turn on sensor
  if (movementSensor.begin() == false)
  {
    Serial.println("Device not found. Check wiring.");
    while (1);
  }
  delay(2000);
}

void loop()
{
  delay(100);
  if (movementSensor.available())
  {
    ir1 = movementSensor.getIR1();
    ir2 = movementSensor.getIR2();
    ir3 = movementSensor.getIR3();
    ir4 = movementSensor.getIR4();

    irsum = ir1 + ir2 + ir3 + ir4 + 4000;

    movementSensor.refresh(); //Read dummy register after new data is read

    //Note: The observable area is shown in the silkscreen.
    //If sensor 2 increases first, the human is on the left
    digitalWrite(RF_VCC, HIGH);

    Serial.print(irsum);
    Serial.println();
    mySwitch.send(address + irsum,32); // 1879048192 legt das Device fest

    digitalWrite(RF_VCC, LOW);
  }

  Serial.flush();
  Serial.end();
  digitalWrite(13,LOW);

  count = 0;
  while(count < 50){
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    power_adc_disable();
    power_spi_disable();
    power_timer0_disable();
    power_timer2_disable();
    power_twi_disable();  
    power_usart0_disable();
    cli(); // deactivate interrupts
    sleep_enable(); // sets the SE (sleep enable) bit
    sei(); // 
    sleep_cpu(); // sleep now!!
    power_all_enable();
    sleep_disable();
  } 
  Serial.begin(9600);
  digitalWrite(13,HIGH);
  movementSensor.reboot();
}

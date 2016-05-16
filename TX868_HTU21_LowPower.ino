/*
  Required libraries:
    - SparkFunHTU21D: https://github.com/sparkfun/SparkFun_HTU21D_Breakout_Arduino_Library

   Hardware setup:
     - TX868 Data pin connected to digital pin 2
     - HTU21D SDA connected to A4 and SCL to A5
     - analog voltage read on pin A0
*/

#include <TempHygroTX868.h>
#include <SparkFunHTU21D.h>

#include <avr/sleep.h>
#include <avr/power.h>

// transmission timer in seconds
#define TXTIMER 300
volatile int nextTxTimer;

// pin of build-in signal LED
#define LED 13
#define RXPIN 2
#define VOLTAGPIN A0

// hard coded address
#define ADDR 0

// uncomment to get debug output at the serial line
//#define DEBUG


HTU21D htu;
TempHygroTX868 tx(RXPIN);

void setup() {

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println();
  Serial.print("Transmission Timer [s]: ");
  Serial.println(TXTIMER);
  Serial.print("Device Address: ");
  Serial.println(ADDR);
  Serial.println();
  Serial.println("Humidity (%)\tTemperature (C)\tVoltage (V)");
#endif

  htu.begin();
  pinMode(LED, OUTPUT);
  nextTxTimer = 0;

  /* Setup the Watchdog timer */
  // clear the reset flag
  MCUSR &= ~(1 << WDRF);
  // enable configuration changes
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout prescaler value to maximum
  WDTCSR = 1 << WDP0 | 1 << WDP3;
  // Enable the watchdog interrupt
  WDTCSR |= _BV(WDIE);
}

void loop() {

  // are we required to send?
  if (nextTxTimer <= 9) {
    // yes - delay remaining time
    delay(nextTxTimer * 1000);
    // reset time to next transmission
    nextTxTimer = TXTIMER;

    tx.setAddress(ADDR);
    tx.setDataType(TempHygroTX868::HTV);

    // read sensor and transmit
    sendData();
  }

  // go into power down sleep
  pwrDownSleep();
}


/*
   Read data from sensor and transmit
*/
void sendData() {

  float humidity = htu.readHumidity();
  float temperature = htu.readTemperature();
  float voltage = analogRead(VOLTAGPIN) / 1024. * 3.30;

#ifdef DEBUG
  Serial.print(humidity);
  Serial.print("\t");
  Serial.print(temperature);
  Serial.print("\t");
  Serial.println(voltage);
#endif

  if (humidity < 900 && temperature < 900 && voltage < 10) {
    // valid reading
    tx.setData(temperature, humidity, voltage);
    digitalWrite(LED, HIGH);
    tx.send();
    digitalWrite(LED, LOW);   
  } else {
    for (byte idx = 0; idx < 10; ++idx) {
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
  }
  
  digitalWrite(LED, LOW);
}

/*
   Interrupt service routine triggered by watchdog
*/
ISR(WDT_vect) {

  // Watchdog oscillator freq is about 116 kHz
  // at 3 V and 25 °C
  // Therefore time between interrupts is
  // 1,048,576 / 116,000 = 9.039 seconds.
  // So decrement the transmission timer by this value.
  nextTxTimer -= 9;
}

/*
   Go into power down mode
   check for more details:
    - http://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html
    - http://www.gammon.com.au/forum/?id=11497
    - http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
*/
void pwrDownSleep() {

  byte adcsra = ADCSRA;
  ADCSRA = 0;  // disable the ADC - saves 80 µA
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  cli(); // stop interrupts
  sleep_bod_disable(); // disable brown-out detection
  sei(); // enable interrupts
  sleep_cpu();

  // At this point the CPU is sleeping until watchdog bites
  sleep_disable();
  power_all_enable();
  ADCSRA = adcsra; // restore ADCSRA
}

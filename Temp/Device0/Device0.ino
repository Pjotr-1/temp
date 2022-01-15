/*
  Simple thermometer with outdoor sensor unit (device0) and indoor display unit (host).
  The data packets are sent from the sensor unit to the display unit using a Gazell network.
  See files:
    wiring_constants.h    E.g. AIN_NO_PS
    wiring_analog.h       E.g. analogReference()
    nrf51.h               E.g. NRF_ADC (NRF_ADC_Type)
    wiring_analog.c       E.g. code for analogRead()

  // ADC reference selection
  #define DEFAULT     ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling  // Use VDD with 1/3 prescaling. (Only applicable when VDD is in the range 2.5V to 3.6V)
  #define VBG         ADC_CONFIG_REFSEL_VBG                       // Use internal 1.2 V band gap reference
  #define VDD_1_3_PS  ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling  // Use VDD with 1/3 prescaling. (Only applicable when VDD is in the range 2.5V to 3.6V)
  // ADC input selection
  #define DEFAULT_INPUT_SEL   ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling     // Analog input pin specified by CONFIG.PSEL with 1/3 prescaling
  #define AIN_NO_PS           ADC_CONFIG_INPSEL_AnalogInputNoPrescaling           // Analog input pin specified by CONFIG.PSEL with no prescaling
  #define VDD_1_3_PS          ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling          // Supply voltage with 1/3 prescaling used as input for the conversion
  // ADC FUNCTIONS
  extern void analogReference( ADC reference selection ) ;
  extern void analogSelection( ADC input selection ) ;
  extern uint32_t analogRead( uint32_t ulPin ) ;
*/

#include <Temp.h>

//#define DEBUG           // Normally undefined
#define TMP36_PIN 1     // Sensor TMP36 AOUT -> GPIO #1

static device_t role = DEVICE0;   // Sensor unit
static device_packet packet;      // Packet sent to Host

void setup() {
  packet.counter = 0;
  packet.ver_major = VER_MAJOR;
  packet.ver_minor = VER_MINOR;
  int retcode = RFduinoGZLL.begin(role);
  debug_setup(retcode);
}

void loop() {
  if (packet.counter < INITIAL_COUNT) {
    // Send packets more often after reset
    RFduino_ULPDelay( MILLISECONDS(PACKET_INTERVAL_INITIAL) );  // NEW in version 2
    //delay(PACKET_INTERVAL_INITIAL);                           // REMOVED in version 2
  } else {
    // Send packets with normal interval
    RFduino_ULPDelay( MILLISECONDS(PACKET_INTERVAL) );          // NEW in version 2
    //delay(PACKET_INTERVAL);                                   // REMOVED in version 2
  }
  delay(WAKEUP_DELAY);                                          // NEW in version 2
  packet.battery = readVdd();
  packet.temp_cpu = RFduino_temperature(CELSIUS);
  packet.sensor_adc = readAnalogGPIO_1();
  packet.counter++;
  (void)RFduinoGZLL.sendToHost((char *)&packet, sizeof(packet));
}

/*
   readVdd: Used by device to check battery level
   Returns: Vdd in mV
*/
short readVdd() {
  analogReference(VBG);         // ADC reference selection. Use internal 1.2V band gap reference.
  analogSelection(VDD_1_3_PS);  // ADC input selection. Supply voltage with 1/3 prescaling used as input for the conversion.
  short adc = analogRead(2);    // The pin has no meaning, it uses VDD pin
  return adc * 3600 / 1023;     // Return VDD in mV
}
/*
  readAnalogGPIO_1: Reads GPIO-1
  Returns: ADC 0-1023
*/
short readAnalogGPIO_1() {
  analogReference(VBG);               // ADC reference selection. Use internal 1.2V band gap reference.
  analogSelection(AIN_NO_PS);         // ADC input selection. Analog input pin specified by CONFIG.PSEL with no prescaling.
  short adc = analogRead(TMP36_PIN);  // GPIO 1, blue wire, 0-1023.
  return adc;
}
/*
   DEBUG PROCEDURES
*/
void debug_setup(int code) {
#ifndef DEBUG
  return;
#endif
  Serial.begin(SERIAL_MONITOR_BAUD_RATE);
  Serial.println("Device0");
  if (code != 0) {
    Serial.print("ErrCode=");
    Serial.println(code);
  }
}

// End of file

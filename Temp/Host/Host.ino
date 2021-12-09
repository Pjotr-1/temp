/*
  Simple thermometer with outdoor sensor unit (device0) and indoor display unit (host).
  The data packets are sent from the sensor unit to the display unit using a Gazell network.
*/

#include <Temp.h>
#include <GyverTM1637.h>

#define DEBUG   // If defined, debug text is sent to the serial monitor.

static int last_rssi = DATA_PACKET_NOT_UPDATED;
static device_t role = HOST;            // Display unit.
static device_packet packet_recv;       // Packet received from the sensor unit (device0).
static int display_counter = 0;         // Number of display updates since restart.
static int packet_counter = 0;          // Time-stamp of received packet.
static int message_counter = 0;         // Number of messages displayed.
static bool packet_valid = false;       // Host received a packet that is still valid (not timed-out).
static bool device_restarted = false;   // Device has been restarted.
static bool host_restarted = true;      // Host has been restarted.
static float cal_temp = 0.0;            // Calibrated temperature in degrees Celsius.
static short sensor_adc = 0;            // Sensor value ADC 0-1023

// TM1637 Display Pin numbers
#define CLK 2   // GPIO2
#define DIO 3   // GPIO3
static GyverTM1637 disp(CLK, DIO);
static const int digit[] = {_0, _1, _2, _3, _4, _5, _6, _7, _8, _9};

void setup() {
  int retcode = RFduinoGZLL.begin(role);
  debug_setup(retcode);
  init_disp();
}

void loop() {
  // Check if a data packet has been received from the sensor unit.
  if (last_rssi != DATA_PACKET_NOT_UPDATED) {
    packet_valid = true;
    packet_counter = 0;
    host_restarted = false;
  }
  // Check if the sensor unit is in initial phase.
  device_restarted = (packet_valid && (packet_recv.counter < INITIAL_COUNT));

  // Check if last received packet is still valid.
  if (packet_counter > PACKET_TIMEOUT_COUNT) {
    packet_valid = false;
    host_restarted = false;
  } else if (device_restarted && (packet_counter > 1)) {
    packet_valid = false;
  }
  // Display temperature and messages (if any)
  cal_temp = convert_A2T(packet_recv.sensor_adc);
  update_display(round(cal_temp));
  debug_printout();

  // Update counters and delay
  display_counter++;
  packet_counter++;
  last_rssi = DATA_PACKET_NOT_UPDATED;
  delay(DISPLAY_INTERVAL - (message_counter * DISPLAY_MSG_TIME));
  message_counter = 0;
}

/*
   onReceive: callback function when the host received a message from device
   Code must be executed quickly to not disturb communication.
*/
void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len) {
  if (role == HOST) {
    if (device == DEVICE0) {
      if (len == sizeof(device_packet)) {
        packet_recv = *(device_packet *)data;
        last_rssi = rssi;
      }
    }
  }
}
/*
   convert_A2T: Converts ADC sensor value received from Device0 to temperature
   Returns: Temperature in Celsius degrees.
*/
float convert_A2T(short adc) {
  // mV = ADC * 1200mV / 1023 (ADC 0-1023)
  // t = (mV - 500) / 10
  // 750mV=25C, 800mV=30C, 850mV=35C, ...
  // 700mV=20C, 650mV=15C, 600mV=10C, 550mV=5C, 500mV=0C, 450mV=-5C, 400mV=-10C, ...
  float mV = adc * 1200 / 1023;
  float temp = (mV - 500) / 10;
  return temp;
}
/*
   DISPLAY PROCEDURES
*/
void init_disp() {
  disp.clear();
  disp.brightness(BRIGHT_DARKEST);      // 0-darkest, 7-brightest
  disp.displayByte(DISPLAY_TEXT_OLGA);  // Just for fun.
  delay(2000);
  disp.displayByte(DISPLAY_TEXT_INIT);
}

void update_display(int temp) {
  // Turn on Colon during initial phase.
  disp.point(device_restarted, true);
  // Display messages, if any.
  if (packet_valid) {
    if (last_rssi < LOW_SIGNAL_RSSI) {
      // Signal Low, distance to Device0 too far
      disp.displayByte(DISPLAY_TEXT_SILO);
      message_counter++;
      delay(DISPLAY_MSG_TIME);
    }
    if (packet_recv.battery < DEVICE_BATTERY_LOW_LEVEL) {
      disp.displayByte(DISPLAY_TEXT_BATT);
      message_counter++;
      delay(DISPLAY_MSG_TIME);
    }
    // Finally, display the temperature.
    if (temp < TOO_LOW_TEMP) {
      disp.displayByte(DISPLAY_TEXT_TOO_LOW);
    } else if (temp > TOO_HIGH_TEMP) {
      disp.displayByte(DISPLAY_TEXT_TOO_HIGH);
    } else if (temp < -9) {
      disp.displayByte(_dash, digit[abs(temp) / 10], digit[abs(temp) % 10], _degree); // -NN^
    } else if (temp < 0) {
      disp.displayByte(_dash, digit[abs(temp)], _degree, _C); // -N^C
    } else if (temp < 10) {
      disp.displayByte(_empty, digit[temp], _degree, _C);// N^C digit[temp],
    } else {
      disp.displayByte(digit[temp / 10], digit[temp % 10], _degree, _C);// NN^C
    }
  } else if (!host_restarted) {
    disp.displayByte(DISPLAY_TEXT_DASH);
  }
}
/*
   DEBUG PROCEDURES
*/
void debug_setup(int code) {
#ifndef DEBUG
  return;
#endif
  Serial.begin(SERIAL_MONITOR_BAUD_RATE);
  Serial.print("Gazell Network: ");
  if (code == 0) {
    Serial.println("OK");
  } else {
    Serial.print("NOT OK, Return Code= ");
    Serial.println(code);
  }
}
void debug_printout() {
#ifndef DEBUG
  return;
#endif
  Serial.println();
  Serial.print("RSSI= "); Serial.println(last_rssi);
  Serial.print("TEMP= "); Serial.println(cal_temp);
  if (last_rssi != 999) {
    if (!(packet_recv.ver_major == VER_MAJOR) || !(packet_recv.ver_minor == VER_MINOR)) {
      Serial.println("ERROR: Device0 must be rebuilt!");
    }
    Serial.print("COUNTER="); Serial.println(packet_recv.counter);
    Serial.print("ADC="); Serial.println(packet_recv.sensor_adc);
    Serial.print("CPU="); Serial.println(packet_recv.temp_cpu);
    Serial.print("BAT="); Serial.print(packet_recv.battery); Serial.println(" mV");
    Serial.print("MAJOR="); Serial.println(packet_recv.ver_major);
    Serial.print("MINOR="); Serial.println(packet_recv.ver_minor);
  }
}

// End of file

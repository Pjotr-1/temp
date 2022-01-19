
/*
  Simple thermometer with outdoor sensor unit (device0) and indoor display unit (host).
  The data packets are sent from the sensor unit to the display unit using a Gazell network.
*/

#include <RFduinoGZLL.h>

#define VER_MAJOR 0   // Version major number
#define VER_MINOR 5   // Version minor number

// Device sensor unit
#define PACKET_INTERVAL 900000UL        // Milliseconds. Normal transmission interval. CHANGED in version 2.1
#define PACKET_INTERVAL_INITIAL 1000    // Milliseconds. Transmission interval before counter reaches INITIAL_COUNT.
#define INITIAL_COUNT 600               // Number of transmissions with interval PACKET_INTERVAL_INITIAL.
#define WAKEUP_DELAY  10                // Milliseconds. Give unit some time to snooze. NEW in version 2.

// Host display unit
#define DISPLAY_INTERVAL 1100           // Milliseconds. Display interval.
#define DISPLAY_MSG_TIME 300            // Milliseconds. Time to display a message.
#define PACKET_TIMEOUT_COUNT 1200       // Packet from sensor unit is not received within the time limit. CHANGED in version 2.1
#define DATA_PACKET_NOT_UPDATED 999     // Packet from sensor unit has not been updated (not received).
#define LOW_SIGNAL_RSSI -90             // Device signal strength level is critical.
#define DEVICE_BATTERY_LOW_LEVEL 2200   // Device battery level is low (mV).
#define TOO_LOW_TEMP -50                // Temperature lower limit.
#define TOO_HIGH_TEMP 50                // Temperature upper limit.
#define DISPLAY_TEXT_INIT _i, _n, _i, _t
#define DISPLAY_TEXT_OLGA _O, _L, _6, _A
#define DISPLAY_TEXT_BATT _b, _a, _t, _t
#define DISPLAY_TEXT_SILO _S, _i, _L, _o
#define DISPLAY_TEXT_TOO_LOW _C, _o, _l, _d
#define DISPLAY_TEXT_TOO_HIGH _H, _l, _6, _H
#define DISPLAY_TEXT_DASH _dash, _dash, _dash, _dash

#define SERIAL_MONITOR_BAUD_RATE 57600  // For debugging purpose only.

// Packet data sent from sensor unit (device) to display unit (host)
typedef struct device_packet {
  short battery;            // 16-bit Vdd (mV)
  short sensor_adc;         // 16-bit ADC 0-1023 from temperature sensor
  float temp_cpu;           // 32-bit CPU temperature (C)
  unsigned int counter;     // 32-bit Counter incremented at every transmission
  byte ver_major;           // 8-bit Version major number
  byte ver_minor;           // 8-bit Version minor number
};
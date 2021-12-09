# temp
Simple project with the Arduino-compatible RFduino for wireless outdoor temperature measurement using the Gazell protocol and the TMP36 sensor.

It consists of one outdoor sensor unit (device0) and one indoor display unit (host). Data packets are sent from the sensor unit to the display unit using the Gazell network.

The sensor unit reads the output voltage of the TMP36 using an analog input ADC value 0-1023. The ADC value is sent to the host, converted to degrees Celsius and displayed. The sensor unit also reads the battery voltage to let the host indicate low battery level. The data packets are sent every 10 minutes (initialy every second). Low signal strength is also indicated by the host. The temperature has been tested with an accurate temperature reference. Accuracy should be within +/- 1 degree C.

The code is dependent of two libraries;
- TM1637 4-digit display (GyverTM1637)
- RFduino library

The file structure might look like;
- Documents/Arduino/libraries
    GyverTM1637/...
    Temp/Temp.h
- Documents/Arduino/Temp
    Device0/Device0.ino
    Host/Host.ino
- Documents/ArduinoData
    RFduino stuff

The RFduino is based on an nRF51-series chip.
For this project chapter 31.Analog to Digital Converter (ADC) is useful.
The RFduino and the TMP36 datasheets are also provided.

Arduino IDE version 1.8.16 (Windows 10)

Host components:
- RFduino RFD22102 DIP
- TM1637 Display
- Battery Pack 2xAA batteries

Sensor components:
- RFduino RFD22301 SMT Module
- Analog Devices TMP36
- 100nF ceramic capacitor
- Battery Pack 2xAAA batteries

Programming of Host and Sensor
- RFduino RFD22121 USB Programmer

NOTE: RFduino has been discontinued and is no longer available 

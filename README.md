# Meteocuadros
Arduino based weather station &amp; PHP frontend

License
Arduino's IDE and many other libraries and utilities are licensed under a Creative Commons Attribution Share-Alike license. Check details here: https://www.arduino.cc/en/Main/FAQ
Any code writed by me for this miniproject is licensed under the same license.

Tips:
- SIM Card must be PIN unlocked.
- 1uF Capacitor is usefull if the power source is not able to provide 1A during SIM800L's communication burts.
- Leds Legend is:
   * Boot sequency during setup is: 
    1. Blink 3 times
    2. Stay ON while sensor setup
    3. Stay OFF until loop() starts
   * Loop sequency is:
    1.  Green ON while reading sensor data
    2.  Green OFF when data collected
    3.  Orange ON while sending data over GPRS
    4.  Orangle OFF when data is sent
    5.  BOTH ON while sleeping to the next measure
    6.  BOTH OFF before next loop

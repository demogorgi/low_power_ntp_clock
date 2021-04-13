# low_power_ntp_clock
Low power NTP-clock based on Wemos D1 Mini, DS1307 clock shield and a waveshare 1.54inch e-Paper

This project features a low power NPT-clock. The general setup is shown on th picture below:
![alt text](./Layout.jpg "Layout with Wemos, clock, e-paper and power bank")

The wireing for the Wemos-setting is as suggested on https://github.com/ZinggJM/GxEPD2/blob/master/ConnectingHardware.md. Except for the BUSY-line, because the D2 pin is needed for the RTC-shield. Fortunately, the BUSY-line is dispensable.

![alt text](./Wireing.jpg "Wireing for the layout.")

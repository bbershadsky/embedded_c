# Example Embedded C code
## Circa 2015-2018 By Boris Bershadsky

Description:
* irbase2.c             - Free running IR remote emulator and cloner
* rot_enc.c             - Calibrated 2P2T relay state machine for precisely controlling peltier module with RTD, 128x64 OLED, and interrupt based rotary encoder
* suit.c                - Multichannel i2c + analog sensors over UART
* suit_final.c          - Analog ADS1115 16ch array with compensated LPM calculation
* uptic.c               - Uptick POST with ESP8266 AP connect && visual OLED RSSI indicator  
* bonus_tsx_tracker.py  - Python BS4 web scraper with PGSQL raw insertion and auto args** re-fetch mechanism
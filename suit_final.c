// Environmental Suit 2.5 main implementation with 16 ADC inputs, flow sensor, and OLED display
// ADDED DEBUG FUNCTION

#include <Wire.h>
#include <Bounce2.h>
#include <Adafruit_ADS1015.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <Flasher.h>

// Initialize OLED display using software SPI
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC	11
#define OLED_CS	12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 32)
#error ("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//Declare ADC addresses on I2C
Adafruit_ADS1115 adsA(0x48);
Adafruit_ADS1115 adsB(0x49);
Adafruit_ADS1115 adsC(0x4A);
Adafruit_ADS1115 adsD(0x4B);

#define BUTTON_PIN 2
#define LED_PIN 7

int ledState = LOW;
float air_volume_liters = 0;
float volume_sample_size = 3;
float flow_rate_m = 0;
int volume_percentage = 0;
unsigned long previousMillis = 0;		// will store last time LED was updated
long OnTime = 250;		                // milliseconds of on-time
long OffTime = 750;		                // milliseconds of off-time

// Instantiate a Bounce object :
Bounce debouncer = Bounce();

// Initialize voltage measurements
int16_t adc0, adc1, adc2, adc3,
		adc4, adc5, adc6, adc7,
		adc8, adc9, adc10, adc11,
		adc12, adc13, adc14, adc15;

float ch0_volt, ch1_volt, ch2_volt, ch3_volt,
	  ch4_volt, ch5_volt, ch6_volt, ch7_volt,
	  ch8_volt, ch9_volt, ch10_volt, ch11_volt,
	  ch12_volt, ch13_volt, ch14_volt, ch15_volt;

//Calibrate flow sensor to match the Omron specifications
float calibrate_flow(void) {
    Serial.println("[..] Calibrating flow sensor");
    float ch11_volt = 0;
    ch11_volt = (adsC.readADC_SingleEnded(3) * 0.1875)/1000;
    Serial.print("[..] Base value: ");
    Serial.println(ch11_volt);
    Serial.println("[..] Expected value: 1.1");
    float flow_multiplier = (1.000000 / ch11_volt);
    Serial.print("[OK] Muliplier set to: ");
    Serial.println(flow_multiplier);
    return flow_multiplier;
}

void debug_all(void) {
	while (digitalRead(BUTTON_PIN)) {
        // Clear the buffer.
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);

        // Calculate and round volume
        float debug_flow = analogRead(A1) * (5.0 / 1023.0);

        //Output to screen
        display.println("DEBUG_FLOW_PRETEST");
        display.setTextSize(2);
        display.print(debug_flow);
        if (debug_flow >= 1.5 && debug_flow < 1.7) {
                digitalWrite(LED_PIN, HIGH);
                display.print(" :o");
        }
        if (debug_flow >= 1.7) {
                display.print(" :O");
                digitalWrite(LED_PIN, HIGH);
        }
        display.display();
        delay(100);
        digitalWrite(LED_PIN, LOW);
	}
}



float init_settings(void) {
    Serial.println("[..] Please set the volume capacity with the potentiometer and press start");

    float sample_test_volume;

    // Flash LED to indicate system is ready to calibrate
    Flasher led1(7, 100, 400);

    // WAIT UNTIL BUTTON IS PRESSED
    while (digitalRead(BUTTON_PIN)) {
        // Clear the buffer.
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);

        // Calculate and round volume
        float volume_pot = analogRead(A0) * (5.0 / 1023.0);
        volume_pot *= 2;
        round(volume_pot);
        volume_pot /= 2;
        //OFFSET VALUE
        volume_pot += 1;

        //Output to screen
        display.println("Lung Volume Capacity");
        display.setTextSize(2);
        display.print("   ");
        display.print(volume_pot, 1);
        display.print("L");
        display.display();

        // Only update the volume if changed to save CPU cycles
        if (sample_test_volume != volume_pot) {
                sample_test_volume = volume_pot;
        }
        led1.Update();
        delay(100);
    }

    Serial.print("[OK] Volume capacity set to ");
    Serial.print(sample_test_volume, 1);
    Serial.println("L");
    display.display();
    return sample_test_volume;
}

void readAll(float sample_test_volume) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    Serial.println("[OK] EXHALE NOW!");
    display.println("EXHALE\nNOW!");
    display.display();

    //Begin calibration of system
    calibrate();

    float liters_per_sample = 0;
    float liters_percent = 0;
    float flow_multiplier = 1; //calibrate_flow();

    // Serial.println("debug before loop");

    // MAIN PROGRAM AREA
    while (liters_per_sample <= sample_test_volume) {
        digitalWrite(LED_PIN, LOW);
        // Read each channel
        adc0 = adsA.readADC_SingleEnded(0);
        adc1 = adsA.readADC_SingleEnded(1);
        adc2 = adsA.readADC_SingleEnded(2);
        adc3 = adsA.readADC_SingleEnded(3);

        adc4 = adsB.readADC_SingleEnded(0);
        adc5 = adsB.readADC_SingleEnded(1);
        adc6 = adsB.readADC_SingleEnded(2);
        adc7 = adsB.readADC_SingleEnded(3);
        
        adc8 = adsC.readADC_SingleEnded(0);
        adc9 = adsC.readADC_SingleEnded(1);
        adc10 = adsC.readADC_SingleEnded(2);
        adc11 = adsC.readADC_SingleEnded(3);
        
        adc12 = adsD.readADC_SingleEnded(0);
        adc13 = adsD.readADC_SingleEnded(1);
        adc14 = adsD.readADC_SingleEnded(2);
        adc15 = adsD.readADC_SingleEnded(3);

        // Convert to voltage using 187.5uV/bit scale factor.
        ch0_volt  = (adc0  * 0.1875)/1000;
        ch1_volt  = (adc1  * 0.1875)/1000;
        ch2_volt  = (adc2  * 0.1875)/1000;
        ch3_volt  = (adc3  * 0.1875)/1000;

        ch4_volt  = (adc4  * 0.1875)/1000;
        ch5_volt  = (adc5  * 0.1875)/1000;
        ch6_volt  = (adc6  * 0.1875)/1000;
        ch7_volt  = (adc7  * 0.1875)/1000;
        
        ch8_volt  = (adc8  * 0.1875)/1000;
        ch9_volt  = (adc9  * 0.1875)/1000;
        ch10_volt = (adc10 * 0.1875)/1000;
        ch11_volt = analogRead(A1) * (5.0 / 1023.0);
        ch11_volt = (adc11 * 0.1875)/1000;

        ch12_volt = (adc12 * 0.1875)/1000;
        ch13_volt = (adc13 * 0.1875)/1000;
        ch14_volt = (adc14 * 0.1875)/1000;
        ch15_volt = (adc15 * 0.1875)/1000;

        //Correct the value with calibration multiplier 
         ch11_volt *= flow_multiplier;

        while (true) {
            // Clear the buffer.
            display.clearDisplay();
        
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
        
            // Calculate and round volume
            float debug_flow = analogRead(A1) * (5.0 / 1023.0);
        
        
            //Output to screen
            display.println("DEBUG2_FLOW");
            display.setTextSize(2);
            display.print(debug_flow);
            display.display();
            delay(100);
        }

        float ch_flow = analogRead(A1) * (5.0 / 1023.0);

        //Verify the user is exhaling to read data

        //ORIG VALUE BEFORE CHANGE
        // if (ch_flow > 1.13) {
        if (ch_flow > 1.5) {
            digitalWrite(LED_PIN, HIGH);

            if (ch_flow > 1.13 && ch_flow < 2.45) {
                    flow_rate_m = ch_flow * 4.082;
            }
            if (ch_flow >= 2.45 && ch_flow < 3.51) {
                    flow_rate_m = ch_flow * 5.698;
            }
            if (ch_flow >= 3.51 && ch_flow < 4.20) {
                    flow_rate_m = ch_flow * 7.143;
            }
            if (ch_flow >= 4.20 && ch_flow < 4.66) {
                    flow_rate_m = ch_flow * 8.584;
            }
            if (ch_flow >= 4.66 && ch_flow < 5.00) {
                    flow_rate_m = ch_flow * 10;
            }

            float flow_rate_lps = flow_rate_m / 60;
             Serial.print(flow_rate_lps);
            //  Serial.print("L/10ms,");
            liters_per_sample += flow_rate_lps;
            //  Serial.print(liters_per_sample);
            //  Serial.print("L (total),");
            liters_percent = (liters_per_sample / sample_test_volume) * 100;

            // Clear the buffer.
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);

            display.println("Sample Progress");
            display.setTextSize(2);
            display.print(liters_per_sample, 1);
            display.print("L/");
            display.print(sample_test_volume);
            display.print("L");
            display.display();

            //TODO FIGURE OUT THIS OFFSET
            //  liters_percent /= 100;

            //Reset the reading
            flow_rate_m = 0;
            delay(100);

            // ADC A
            Serial.print(ch0_volt, 7); Serial.print(",");
            Serial.print(ch1_volt, 7); Serial.print(",");
            Serial.print(ch2_volt, 7); Serial.print(",");
            Serial.print(ch3_volt, 7); Serial.print(",");
            // ADC B
            Serial.print(ch4_volt, 7); Serial.print(",");
            Serial.print(ch5_volt, 7); Serial.print(",");
            Serial.print(ch6_volt, 7); Serial.print(",");
            Serial.print(ch7_volt, 7); Serial.print(",");
            // ADC C
            Serial.print(ch8_volt, 7); Serial.print(",");
            Serial.print(ch9_volt, 7); Serial.print(",");
            Serial.print(ch10_volt, 7); Serial.print(",");
            Serial.print(ch11_volt, 7); Serial.print(",");
            // ADC D
            Serial.print(ch12_volt, 7); Serial.print(",");
            Serial.print(ch13_volt, 7); Serial.print(",");
            Serial.print(ch14_volt, 7); Serial.print(",");
            Serial.print(ch15_volt, 7); Serial.print(",");

            //ARDUINO MEGA VOLTAGE READING APPROXIMATIONS
            float ch02_volt = analogRead(A2) * (5.0 / 1023.0);
            float ch03_volt = analogRead(A3) * (5.0 / 1023.0);
            float ch04_volt = analogRead(A4) * (5.0 / 1023.0);
            float ch05_volt = analogRead(A5) * (5.0 / 1023.0);
            float ch06_volt = analogRead(A6) * (5.0 / 1023.0);
            float ch07_volt = analogRead(A7) * (5.0 / 1023.0);
            float ch08_volt = analogRead(A8) * (5.0 / 1023.0);
            float ch09_volt = analogRead(A9) * (5.0 / 1023.0);
            float ch10_volt = analogRead(A10) * (5.0 / 1023.0);
            float ch11_volt = analogRead(A11) * (5.0 / 1023.0);
            float ch12_volt = analogRead(A12) * (5.0 / 1023.0);
            float ch13_volt = analogRead(A13) * (5.0 / 1023.0);
            float ch14_volt = analogRead(A14) * (5.0 / 1023.0);
            float ch15_volt = analogRead(A15) * (5.0 / 1023.0);

            //PRINT TO SERIAL
            Serial.print(ch02_volt, 7); Serial.print(",");
            Serial.print(ch03_volt, 7); Serial.print(",");
            Serial.print(ch04_volt, 7); Serial.print(",");
            Serial.print(ch05_volt, 7); Serial.print(",");
            Serial.print(ch06_volt, 7); Serial.print(",");
            Serial.print(ch07_volt, 7); Serial.print(",");
            Serial.print(ch08_volt, 7); Serial.print(",");
            Serial.print(ch09_volt, 7); Serial.print(",");
            Serial.print(ch10_volt, 7); Serial.print(",");
            Serial.print(ch11_volt, 7); Serial.print(",");
            Serial.print(ch12_volt, 7); Serial.print(",");
            Serial.print(ch13_volt, 7); Serial.print(",");
            Serial.print(ch14_volt, 7); Serial.print(",");
            Serial.print(ch15_volt, 7); Serial.print(",");

            // Flow rate
            Serial.print(liters_per_sample); Serial.print(",");
            // Serial.print(flow_rate_m); Serial.print(",");
            //  Serial.print("L/m,");
            Serial.print(liters_percent);

            // End the line
            Serial.print('\n');
            }
		}
		Serial.println("[OK] Sample test complete");
		display.clearDisplay();
		display.setTextSize(2);
		display.setTextColor(WHITE);
		display.setCursor(0,0);
		display.println("TEST");
		display.println("COMPLETE");
		display.display();

// Turn off LED
		digitalWrite(LED_PIN,LOW);
}
void setup(void) {
    // Setup serial connection
    Serial.begin(115200);

    // Initialize OLED display
    display.begin(SSD1306_SWITCHCAPVCC);

    Serial.println("[OK] Suit 2.5 Initialized, warming up ADC arrays");

    // Setup the action button with an internal pull-up and Bounce instance
    pinMode(BUTTON_PIN,INPUT_PULLUP);
    debouncer.attach(BUTTON_PIN);
    debouncer.interval(50);

    // Setup the LED and state
    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN,ledState);

    // Initialize ADC array
    adsA.begin();
    adsB.begin();
    adsC.begin();
    adsD.begin();

    debug_all();
    delay(1000);
    readAll(init_settings());
    test_led();
    calibrate_flow();
    read_flow();
    test_button();
}

void loop() {
}

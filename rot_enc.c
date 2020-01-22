#include <Adafruit_MAX31865.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//these pins can not be changed 2/3 are special pins
int encoderPin1 = 2;
int encoderPin2 = 3;
int encoderSwitchPin = 4;	 //push button switch
int relay_A = 5;	// main heating relay for temperature sensor
int relay_B = 6; // main cooling relay for temperature sensor
int relay_C = 7; // Fan relay

volatile int lastEncoded = 0;
// volatile long encoderValue = 0; //orig value
// volatile long encoderValue = 1466; // 36.6 C
volatile long encoderValue = 1360; // 34.0 C

long lastencoderValue = 1360;

int lastMSB = 0;
int lastLSB = 0;
float curr_temp = 0;
float set_temp = 34;
float max_temp = 0;
// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 max = Adafruit_MAX31865(10, 11, 12, 13);

// The value of the Rref resistor. Use 430.0!
// #define RREF 429.73
#define RREF 432.4
// #define RREF 430

void setup() {
	Serial.begin (115200);
	max.begin(MAX31865_3WIRE);	// set to 2WIRE or 4WIRE as necessary

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);	// initialize with the I2C addr 0x3C (for the 128x32)
	// display.display();
	// display.clearDisplay();

	pinMode(encoderPin1, INPUT);
	pinMode(encoderPin2, INPUT);
	pinMode(encoderSwitchPin, INPUT);
	pinMode(relay_A, OUTPUT);
	pinMode(relay_B, OUTPUT);

	digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
	digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
	digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on

	//call updateEncoder() when any high/low changed seen
	//on interrupt 0 (pin 2), or interrupt 1 (pin 3)
	attachInterrupt(0, updateEncoder, CHANGE);
	attachInterrupt(1, updateEncoder, CHANGE);
}

void loop(){
	uint16_t rtd = max.readRTD();
    Serial.print("RTD value =	"); Serial.println(rtd);
    float ratio = rtd;
    ratio /= 32768;
    Serial.print("Ratio =			"); Serial.println(ratio,8);
    Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
    Serial.print("Temp			 = "); Serial.println(max.temperature(100, RREF), 6);
    curr_temp = max.temperature(100, RREF);
    if (curr_temp < set_temp) {
        digitalWrite(relay_B, LOW);
        digitalWrite(relay_A, HIGH);
    } else {
    digitalWrite(relay_A, LOW);
    digitalWrite(relay_B, HIGH);
    }

    if (curr_temp > max_temp) {
        max_temp = curr_temp;
    }
    Serial.print("MAX					="); Serial.println(max_temp);
    // Serial.print("Temperature = "); Serial.println(max.temperature(100, RREF));

    // Check and print any faults
    uint8_t fault = max.readFault();
    if (fault) {
        Serial.print("Fault 0x"); Serial.println(fault, HEX);
        if (fault & MAX31865_FAULT_HIGHTHRESH) {
            Serial.println("RTD High Threshold");
        }
        if (fault & MAX31865_FAULT_LOWTHRESH) {
            Serial.println("RTD Low Threshold");
        }
        if (fault & MAX31865_FAULT_REFINLOW) {
            Serial.println("REFIN- > 0.85 x Bias");
        }
        if (fault & MAX31865_FAULT_REFINHIGH) {
            Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
        }
        if (fault & MAX31865_FAULT_RTDINLOW) {
            Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
        }
        if (fault & MAX31865_FAULT_OVUV) {
            Serial.println("Under/Over voltage");
        }
        max.clearFault();
    }

	if (digitalRead(encoderSwitchPin)) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.print("NOW:");
        display.print(curr_temp);
        display.println();
        display.print("SET:");
        set_temp = encoderValue/4;
        set_temp = set_temp / 10;
        display.print(set_temp);
        display.display();
	} else {
		//button is being pushed
		Serial.println("[BUTTON]");
	}
	// DEBUG ENCODER VALUE
	Serial.println(encoderValue);
}

void updateEncoder(){
	int MSB = digitalRead(encoderPin1); //MSB = most significant bit
	int LSB = digitalRead(encoderPin2); //LSB = least significant bit

	int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
	int sum	= (lastEncoded << 2) | encoded; //adding it to the previous encoded value

	if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
	if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

	lastEncoded = encoded; //store this value for next time
}

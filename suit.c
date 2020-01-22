#define DHTPIN A2		 // DHT11 Temphum sensor
#define DHTTYPE DHT11 // DHT11
#define heaterSelPin 8 //Ethanol

#include "Barometer.h"
#include "MutichannelGasSensor.h"
#include <Wire.h>
#include <math.h>
#include "DHT.h"

float temperatureC;	    //Celcius
float temperatureF;	    //Fahrenheit
float temperatureK;	    //Kelvin

float sensorValue;		//O2 sensor
float sensorVoltage;	//O2 sensor
float Value_O2;			//O2 sensor

float pressureP;	    //Pressure
float pressureMB;		//Pressure (mBar)
float altitude;			//Altitude in m
float atm;				//Atm

float sensor_volt;		//MQ3
float sensorValueMQ3;	//MQ3

const int buttonPin = 9;		    // the number of the pushbutton pin
const int ledPin = 4;				// the number of the LED pin

int buttonState = 0;				// variable for reading the pushbutton status

DHT dht(DHTPIN, DHTTYPE);

//Warning values
float WarningValue= 19.5;	//The minimum allowed concentration of O2 in air	

Barometer myBarometer;

void setup(){
	//Multichannel Sensor
	mutichannelGasSensor.begin(0x04);   //the default I2C address of the slave is 0x04
	mutichannelGasSensor.powerOn();
	Serial.begin(9600);
	myBarometer.init();
	dht.begin();
	pinMode(heaterSelPin,OUTPUT);	    // set the heaterSelPin as digital output.
	digitalWrite(heaterSelPin,LOW);     // Start to heat the sensor	

	//Initialize the LED pin as an output:
	pinMode(ledPin, OUTPUT);			
	//Initialize the pushbutton pin as an input:
	pinMode(buttonPin, INPUT);		
	//Headers
	// Serial.println("[A2] Humidity: ,[A0] MQ3: ,[I2C 1] Temp *C ,[I2C 1] Pressure (Pa):	,Pressure (mBr): ,[I2C 1] Relative Atmosphere (atm): ,[I2C 1] Altitude (m): ,[A3] O2 concentration (%): ,[I2C 2] Ammonia (ppm):	,[I2C 2] CO (ppm): ,[I2C 2] NO2 (ppm): ,[I2C 2] Propane (ppm): ,[I2C 2] Butane (ppm): ,[I2C 2] Methane (ppm): ,[I2C 2] H2 (ppm): ,[I2C 2] Ethanol (ppm):"); 
}

void loop() {
	//DHT11 Temperature/Humidity
	float h = dht.readHumidity();
	float t = dht.readTemperature();
	// check if returns are valid, if they are NaN (not a number) then something went wrong!
	if (isnan(t) || isnan(h)) {
		Serial.println("Failed to read from DHT");
	} else {
        Serial.print("[A2] Humidity: ");
		Serial.print(",");
		Serial.print(h);
		Serial.print(",");
		//DHT11 temp omitted
		}
	
	//Barometer I2C 1
	temperatureC = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
	temperatureF = (temperatureC * (9.0/5.0)) + 32;
	temperatureK = temperatureC + 273.15;
	pressureP = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP());//Get the temperature
	pressureMB = pressureP / 100;
	altitude = myBarometer.calcAltitude(pressureP); //Uncompensated caculation in meters
	atm = pressureP / 101325;

	//MQ3
	sensorValue = analogRead(A0);       //MQ3 sensor
	sensor_volt = sensorValueMQ3/1024*5.0;
	Serial.print("[A0] MQ3:");
	Serial.print(",");
	Serial.print(sensorValue);
	Serial.print(",");

	//Multichannel sensor
	Serial.print("[I2C 1] Temp *C");
	Serial.print(",");
	Serial.print(temperatureC, 2);      //display 2 decimal places
	Serial.print(",");
	
    Serial.print("C / ");
    Serial.print(",");
    Serial.print(temperatureF, 2);
    Serial.print("F / ");
    Serial.print(",");
    Serial.print(temperatureK, 2);
    Serial.println("K");

	Serial.print("[I2C 1] Pressure (Pa): ");
	Serial.print(",");
	Serial.print(pressureP, 0);
	Serial.print(",");
	Serial.print("Pressure (mBr): ");
	Serial.print(",");
	Serial.print(pressureMB);
	Serial.print(",");

	Serial.print("[I2C 1] Rel. Atmosphere (atm): ");
	Serial.print(",");
	Serial.print(atm, 4);       //display 4 decimal places
	Serial.print(",");

	Serial.print("[I2C 1] Altitude (m): ");
	Serial.print(",");
	Serial.print(altitude, 2);  //display 2 decimal places
	Serial.print(",");

	//O2 Calculate
	sensorValue = analogRead(A3);
	sensorVoltage =(sensorValue/1024)*5.0;
	sensorVoltage = sensorVoltage/201*10000;
	Value_O2 = sensorVoltage/7.43;
	Serial.print("[A3] O2 concentration (%): ");
	Serial.print(",");
	Serial.print(Value_O2,1);
	Serial.print(",");
 
	if(Value_O2<=WarningValue) {
		digitalWrite(3,HIGH);
	}
	else digitalWrite(3,LOW);

	//Multichannel Gas
	float c1;
	c1 = mutichannelGasSensor.measure_NH3();
	Serial.print("[I2C 2] Ammonia (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");

	c1 = mutichannelGasSensor.measure_CO();
	Serial.print("[I2C 2] CO (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");

	c1 = mutichannelGasSensor.measure_NO2();
	Serial.print("[I2C 2] NO2 (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");

	c1 = mutichannelGasSensor.measure_C3H8();
	Serial.print("[I2C 2] Propane (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");
	
	c1 = mutichannelGasSensor.measure_C4H10();
	Serial.print("[I2C 2] Butane (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");

	c1 = mutichannelGasSensor.measure_CH4();
	Serial.print("[I2C 2] Methane (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");

	c1 = mutichannelGasSensor.measure_H2();
	Serial.print("[I2C 2] H2 (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.print(c1);
	else Serial.print("invalid");
	Serial.print(",");
	
	c1 = mutichannelGasSensor.measure_C2H5OH();
	Serial.print("[I2C 2] Ethanol (ppm): ");
	Serial.print(",");
	if(c1>=0) Serial.println(c1);
	else Serial.print("invalid");
	Serial.println(",");

	//Ethanol
    float sensor_voltETH; 
    float RS_airETH;                        //	Get the value of RS via in a clear air
    float sensorValueETH;

    for(int x = 0 ; x < 100 ; x++) {        // Test 100 times and calculate avg
        sensorValueETH = sensorValueETH + analogRead(A2);
    }
    sensorValueETH = sensorValueETH/100.0;

    sensor_voltETH = sensorValueETH/1024*5.0;
    RS_airETH = sensor_voltETH/(5.0-sensor_voltETH); // omit *R16
    Serial.print("[A1] Ethanol Voltage: ");
    Serial.print(sensor_voltETH);
    Serial.println("V");
    Serial.print("[A1] Ethanol Value: ");
    Serial.println(RS_airETH);
	
	Serial.println("********************************************************");
	delay(1000);

	// read the state of the pushbutton value:
	buttonState = digitalRead(buttonPin);

	// check if the pushbutton is pressed.
	// if it is, the buttonState is HIGH:
	if (buttonState == HIGH) {		 
		// turn LED on:		
		Serial.println("***RESET***");
		digitalWrite(ledPin, HIGH);	
	} 
	else {
		// turn LED off:
		digitalWrite(ledPin, LOW); 
	}
}

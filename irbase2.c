#include <IRremote.h>
#include <LiquidCrystal.h>

IRsend irsend;
LiquidCrystal lcd(12, 11, 5, 4, 6, 2);
//11 12 13 14
//4	 5	6	7

//Create a receiver object to listen on pin 11
IRrecv My_Receiver(11);
IRsend My_Sender; //pin 3

//Create a decoder object
//IRdecode My_Decoder;

const int button = 13;
const int led = 13;

int bounceTime = 50;
int holdTime = 250;
int doubleTime = 500;

int lastReading = LOW;
int hold = 0;
int single = 0;
int LEDstate = 0;

long onTime = 0;
long lastSwitchTime = 0;

void setup()
{
	lcd.begin(16, 2);
	lcd.print("HAPPYSRVR INIT");
	delay(1000);
	pinMode(button, INPUT);
	lcd.clear();
	Serial.begin(9600);
	My_Receiver.enableIRIn(); // Start the receiver
}

void loop() {
//Serial.println("reciving data");

lcd.setCursor(0,0);
lcd.print("HAPPYSRVR READY");
	int reading = digitalRead(button);

//first pressed
 if (reading == HIGH && lastReading == LOW) {
	 onTime = millis();
 }

//held
 if (reading == HIGH && lastReading == HIGH) {
	 if ((millis() - onTime) > holdTime) {
//		 invertLED();	
		 hold = 1;
	 }
 }

//released
 if (reading == LOW && lastReading == HIGH) {
	 if (((millis() - onTime) > bounceTime) && hold != 1) {
		 onRelease();
	 }
	 if (hold == 1) {
		 Serial.println("held - Power OFF");
		 irsend.sendNEC(0xC728D, 20); //Power OFF
		 delay(1000);
		 irsend.sendNEC(0xC728D, 20); //Power OFF
		 hold = 0;
	 }	 
 }
 lastReading = reading;

 if (single == 1 && (millis() - lastSwitchTime) > doubleTime) {
	 Serial.println("single press - Power ON");
	 irsend.sendNEC(0xCF20D, 20);
	 single = 0;
 }
}

void onRelease() {
 if ((millis() - lastSwitchTime) >= doubleTime) {
	 single = 1;
	 lastSwitchTime = millis();
	 return;
 }	

 if ((millis() - lastSwitchTime) < doubleTime) {
	 //toggleLED();
	 Serial.println("double press");
	 single = 0;
	 lastSwitchTime = millis();
 }	
}

// TODO PUT INTO SEPARATE FILE
 //if (buttonState == HIGH) {
	 // My_Sender.send(NEC, 0xC728D, 20); //Power ON
 // }
//Continuously look for results. When you have them pass them to the decoder
//	if (My_Receiver.GetResults(&My_Decoder)) {
	//	My_Decoder.decode();		//Decode the data
	 // My_Decoder.DumpResults(); //Show the results on serial monitor
	 // My_Receiver.resume();		 //Restart the receiver
 // if (Serial.read() != -1) {	
	//	My_Sender.send(NEC, 0xC728D, 20); //Power OFF


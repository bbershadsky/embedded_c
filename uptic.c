// UpTick v0.1 2016-10-04
#include <Wire.h>	// Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <ESP8266WiFi.h>

// Include custom images
// #include "images.h"

// Initialize the OLED display using Wire library
SSD1306	display(0x3c, 0, 2); //D3, D4 on ESP8266 WEMOS
const char* wifi_ssid = "SECRET";
#define wifi_password SECRET
char servername[]="tradyboty.club";	// remote server we will connect to


void drawInstructions() {
  display.clear();
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawStringMaxWidth(0, 0, 128,
		"UpTick is a standalone currency ticker that displays real-time prices\n												http://uptick.io/setup");
	display.display();
	delay(3000);
	display.clear();
	display.drawStringMaxWidth(0, 0, 128,
		"Please connect your UpTick to your WiFi to get started \n																		http://uptick.io/setup");
	display.display();
	delay(3000);

	// display.clear();
	// display.drawProgressBarDemo();
	// display.display();
	}


void printBuffer(void) {
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  display.setLogBuffer(5, 30);

  // Some test data
  const char* test[] = {
      "Hello",
      "Uptick" ,
      "----",
      "Show off",
      "how",
      "the log buffer",
      "is",
      "working.",
      "Even",
      "scrolling is",
      "working"
  };

  for (uint8_t i = 0; i < 11; i++) {
    display.clear();
    // Print to the screen
    display.println(test[i]);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(500);
    display.clear();
	}
}

int connectToWifi() {
	const char* ssid		 = "SECRET";
	const char* password = "SECRET";
	WiFi.begin(ssid, password); //OVERRIDE
	Serial.print("Connecting to " + wifi_ssid);
	display.setFont(ArialMT_Plain_10);
	display.invertDisplay();

  printBuffer();

	while (WiFi.status() != WL_CONNECTED)
	{
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
		drawInstructions();
		display.display();
		delay(500);
		Serial.print(".");
		display.setTextAlignment(TEXT_ALIGN_CENTER);
		display.drawString(64, 22, "CONNECTING...");
		display.drawString("CONNECTING!!!");
	}
	Serial.println();

	display.clear();
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 24, "WiFi CONNECTED!");
	display.display();
	delay(1000);

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());
	return WiFi.RSSI();
}

void displayWifiStrength(long RSSI) {
	Serial.println(RSSI);
}

void setup() {
	Serial.begin(115200);
	display.init();
	displayWifiStrength(connectToWifi());
}


void drawTextAlignmentDemo() {
		// Text alignment demo
	display.setFont(ArialMT_Plain_10);

	// The coordinates define the left starting point of the text
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 10, "Left aligned (0,10)");

	// The coordinates define the center of the text
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.drawString(64, 22, "Center aligned (64,22)");

	// The coordinates define the right end of the text
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(128, 33, "Right aligned (128,33)");
}

void drawRectDemo() {
			// Draw a pixel at given position
		for (int i = 0; i < 10; i++) {
			display.setPixel(i, i);
			display.setPixel(10 - i, i);
		}
		display.drawRect(12, 12, 20, 20);

		// Fill the rectangle
		display.fillRect(14, 14, 17, 17);

		// Draw a line horizontally
		display.drawHorizontalLine(0, 40, 20);

		// Draw a line horizontally
		display.drawVerticalLine(40, 0, 20);
}

void drawProgressBarDemo() {
	int progress = (counter / 5) % 100;
	// draw the progress bar
	display.drawProgressBar(0, 32, 120, 10, progress);

	// draw the percentage as String
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.drawString(64, 15, String(progress) + "%");
}

void drawImage() {

		display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

void drawSignalStrength() {
	long rssi = WiFi.RSSI();
	// 30-45, 45-60, 50-65, 65-70, 75-90
	//		 0,		 1,		 2,		 3,		 4

	Serial.print("RSSI:");
	Serial.println(rssi);

	display.clear();
	display.drawString(64, 38, String(rssi));
	display.display();
}

void loop() {
	drawSignalStrength();
	delay(1000);
}
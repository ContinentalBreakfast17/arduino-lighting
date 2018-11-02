#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "lighting.hpp"

// function declarations
void initFastLed();
bool pinIsValid(int pin);
CLEDController& getFastLed(int pin);
int parseCommand();
byte parseStaticConfig(StaticConfig& config);
byte parseAddressableConfig(AddressableConfig& config);
byte saveAddressableConfig(AddressableConfig& config);
void displayStatic(StaticConfig& config);
void displayAddressable(AddressableConfig& config);
void respond(byte code);

// static error codes
#define ERR_S_JSON_READ_FAILURE  1
#define ERR_S_NO_MODE            2
#define ERR_S_NO_SPEED           3
#define ERR_S_NO_COLOR           4
#define ERR_S_NO_PINS            5
#define ERR_S_BAD_MODE           6
#define ERR_S_BAD_SPEED          7
#define ERR_S_BAD_PIN_LENGTH     8
#define ERR_S_BAD_PINS           9
#define ERR_S_BAD_COLOR_LENGTH   10
#define ERR_S_BAD_COLOR          11
// addressable error codes
#define ERR_A_JSON_READ_FAILURE  12
#define ERR_A_MISSING_PIN        13
#define ERR_A_MISSING_SPEED      14
#define ERR_A_MISSING_LED_COUNT  15
#define ERR_A_MISSING_SEQ_SIZE   16
#define ERR_A_BAD_PIN            17
#define ERR_A_FILE_OPEN_FAILURE  18
#define ERR_A_SERIAL_TIMEOUT     19
#define ERR_A_FILE_WRITE_FAILURE 20

#define SERIAL_READ_TRIES 5
#define SERIAL_RETRY_WAIT 5000

// test message:
// 1{"mode":0,"speed":5,"pins":[9,10,11],"color":[100,0,200]}
// A[{"pin":2,"speed":30,"ledCount":16,"seqSize":2}]
// [[128,128,128],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0]]
// [[0,0,0],[128,128,128],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0],[0,0,0]]


// Global controller which holds the current data
RGBController controller;

bool pinIsValid(int pin) {
	switch(pin) {
		case 4:
			return true;
		default:
			return false;
	}
}

CLEDController& getFastLed(int pin) {
	switch(pin) {
		case 4:
			return FastLED[0];
		default:
			return FastLED[0];
	}
}

void initFastLed() {
	FastLED.addLeds<WS2811, 4>(NULL, 0);
}

void setup () {
	initFastLed();

	Serial.begin(9600);
	if(!SD.begin(53)) Serial.println("fuck");

	Channel channel(4, 1000, 16, 2);
	CLEDController& strip = getFastLed(channel.pin);
	strip.setLeds(channel.leds, channel.ledCount);
	AddressableConfig& config = controller.getAddressable();
	config.addChannel(channel);
	config.on = 1;
}

void loop() {
	int parseResult = parseCommand();
	if(parseResult >= 0) respond(parseResult);

	displayStatic(controller.getStatic());
	displayAddressable(controller.getAddressable());
}

int parseCommand() {
	if(!Serial.available()) return -1;

	byte result = 0;
	char code = Serial.read();
	if(code == '1') {
		if(result = parseStaticConfig(controller.getStatic())) return result;
	} else {
		if(result = parseAddressableConfig(controller.getAddressable())) return result;
		if(result = saveAddressableConfig(controller.getAddressable())) return result;
	}

	return 0;
}

byte parseStaticConfig(StaticConfig& config) {
	DynamicJsonBuffer jsonBuffer(128);
	JsonObject& sttc = jsonBuffer.parseObject(Serial);
	if(!sttc.success()) return ERR_S_JSON_READ_FAILURE;

	if(!sttc.containsKey("mode")) return ERR_S_NO_MODE;
	if(!sttc.containsKey("speed")) return ERR_S_NO_SPEED;
	if(!sttc.containsKey("pins")) return ERR_S_NO_PINS;
	if(!sttc.containsKey("color")) return ERR_S_NO_COLOR;

	if(config.setMode(sttc["mode"])) return ERR_S_BAD_MODE;
	if(config.setSpeed(sttc["speed"])) return ERR_S_BAD_SPEED;

	byte pins[3];
	if(sttc.get<JsonArray&>("pins").copyTo(pins) != 3) return ERR_S_BAD_PIN_LENGTH;
	if(config.setPins(pins)) return ERR_S_BAD_PINS;
	for(byte i = 0; i < 3; i++) pinMode(pins[i], OUTPUT);

	byte color[3];
	if(sttc.get<JsonArray&>("color").copyTo(color) != 3) return ERR_S_BAD_COLOR_LENGTH;
	if(config.setColor(color)) return ERR_S_BAD_COLOR;

	config.on = 1;
	return 0;
}

byte parseAddressableConfig(AddressableConfig& config) {
	DynamicJsonBuffer jsonBuffer(512);
	JsonArray& chans = jsonBuffer.parseArray(Serial);
	if(!chans.success()) return ERR_A_JSON_READ_FAILURE;
	config.clear();

	for(JsonObject& chan : chans) {
		if(!chan.containsKey("pin")) return ERR_A_MISSING_PIN;
		if(!chan.containsKey("speed")) return ERR_A_MISSING_SPEED;
		if(!chan.containsKey("ledCount")) return ERR_A_MISSING_LED_COUNT;
		if(!chan.containsKey("seqSize")) return ERR_A_MISSING_SEQ_SIZE;
		if(!pinIsValid(chan["pin"])) return ERR_A_BAD_PIN;
		Channel channel(chan["pin"], chan["speed"], chan["ledCount"], chan["seqSize"]);
		CLEDController& strip = getFastLed(channel.pin);
		strip.setLeds(channel.leds, channel.ledCount);
		config.addChannel(channel);
	}
	return 0;
}

byte saveAddressableConfig(AddressableConfig& config) {
	for(int channelIndex = 0; channelIndex < config.channelCount; channelIndex++) {
		Channel& channel = config.getChannel(channelIndex);

		for(int cycleIndex = 0; cycleIndex < channel.seqSize; cycleIndex++) {
			char filename[9];
			int n = sprintf(filename, "%d-%d", channelIndex, cycleIndex);
			SD.remove(filename);
			File file = SD.open(filename, FILE_WRITE);
			if(!file) return ERR_A_FILE_OPEN_FAILURE;
			Serial.println("opened a file");

			char serialReadTries = 0, c = 0;
			while(c != '\r') {
				if(!Serial.available()) {
					if(serialReadTries > SERIAL_READ_TRIES) {
						file.close();
						return ERR_A_SERIAL_TIMEOUT;
					}
					serialReadTries++;
					delay(SERIAL_RETRY_WAIT);
					continue;
				}

				c = Serial.read();
				if(c != '\r' && file.write(c) != 1) {
					file.close();
					return ERR_A_FILE_WRITE_FAILURE;
				}
				serialReadTries = 0;
			}
			file.close();
		}
	}
	config.on = 1;
	return 0;
}

void displayStatic(StaticConfig& config) {
	if(!config.on) return;
	long long now = millis();
	if(config.shouldDisplay(now)) {
		byte* color = config.getDisplayColor();
		byte* pins = config.getPins();
		for(byte i = 0; i < 3; i++) analogWrite(pins[i], color[i]);
	}
}

void displayAddressable(AddressableConfig& config) {
	if(!config.on) return;

	for(byte channelIndex = 0; channelIndex < config.channelCount; channelIndex++) {
		Channel& channel = config.getChannel(channelIndex);
		long long now = millis();

		if(channel.shouldDisplay(now)) {
			int cycleIndex = channel.getIndex();
			if(!pinIsValid(channel.pin)) continue;
			CLEDController& strip = getFastLed(channel.pin);

			char filename[9];
			sprintf(filename, "%d-%d", channelIndex, cycleIndex);
			File file = SD.open(filename, FILE_READ);
			if(!file) continue; // Do something else?

			for(int ledIndex = 0; ledIndex < channel.ledCount; ledIndex++) {
				for(int colorIndex = 0; colorIndex < 3; colorIndex++) {
					channel.leds[ledIndex][colorIndex] = file.parseInt();
				}
				//Serial.print(int(leds[ledIndex][0])); Serial.print(int(leds[ledIndex][1])); Serial.print(int(leds[ledIndex][2])); Serial.print('\n');
			}
			FastLED.show();
			file.close();
		}
	}
}

void respond(byte code) {
	Serial.print(code);
}
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "lighting.hpp"

typedef union SerialLong{
	unsigned long l;
	byte          bytes[4];
}SerialLong;

// function declarations
void initFastLed();
bool pinIsValid(int pin);
CLEDController& getFastLed(int pin);
void initConfig();
int parseCommand();
byte parseStatic(StaticConfig& config, Stream& stream);
byte parseAddressable(AddressableConfig& config, Stream& stream, bool isInit);
byte parseAddressableConfig(AddressableConfig& config, char* in, bool isInit);
byte parseAddressableData(AddressableConfig& config);
void displayStatic(StaticConfig& config);
void displayAddressable(AddressableConfig& config);
SerialLong parseLong(Stream& stream);
bool available(Stream& stream);
void respond(byte code);

// static error codes
#define ERR_S_CFG_READ_FAILURE   1
#define ERR_S_JSON_READ_FAILURE  2
#define ERR_S_NO_MODE            3
#define ERR_S_NO_SPEED           4
#define ERR_S_NO_COLOR           5
#define ERR_S_NO_PINS            6
#define ERR_S_BAD_MODE           7
#define ERR_S_BAD_SPEED          8
#define ERR_S_BAD_PIN_LENGTH     9
#define ERR_S_BAD_PINS           10
#define ERR_S_BAD_COLOR_LENGTH   11
#define ERR_S_BAD_COLOR          12
#define ERR_S_CFG_SAVE_FAILURE   13
// addressable error codes
#define ERR_A_CFG_READ_FAILURE   14
#define ERR_A_JSON_READ_FAILURE  15
#define ERR_A_MISSING_PIN        16
#define ERR_A_MISSING_SPEED      17
#define ERR_A_MISSING_LED_COUNT  18
#define ERR_A_MISSING_SEQ_SIZE   19
#define ERR_A_BAD_PIN            20
#define ERR_A_CFG_SAVE_FAILURE   21
#define ERR_A_FILE_OPEN_FAILURE  22
#define ERR_A_SERIAL_TIMEOUT     23
#define ERR_A_FILE_WRITE_FAILURE 24
#define ERR_A_EOF                25

#define SERIAL_READ_TRIES 5
#define SERIAL_RETRY_WAIT 100

// Global controller which holds the current data
RGBController controller;

// These are necessary unfortunately...
bool pinIsValid(int pin) {
	switch(pin) {
		case 4:
		case 3:
			return true;
		default:
			return false;
	}
}

CLEDController& getFastLed(int pin) {
	switch(pin) {
		case 4:
			return FastLED[0];
		case 3:
			return FastLED[1];
		default:
			return FastLED[0];
	}
}

void initFastLed() {
	FastLED.addLeds<WS2811, 4, GRB>(NULL, 0);
	FastLED.addLeds<WS2811, 3, GRB>(NULL, 0);
}

void setup () {
	initFastLed();

	Serial.begin(115200);
	//Serial.begin(9600, SERIAL_8E2);
	if(!SD.begin(53)) respond(255);

	initConfig();
	respond(0);
}

void initConfig() {
	File sttc = SD.open(F("sttc"), FILE_READ);
	int result = 0;
	if(sttc) {
		if(result = parseStatic(controller.getStatic(), sttc, true)) respond(result);
		sttc.close();
	}

	File addr = SD.open(F("addr"), FILE_READ);
	if(addr) {
		if(result = parseAddressable(controller.getAddressable(), addr, true)) respond(result);
		addr.close();
	}
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

	switch(code) {
	case 's':
		if(result = parseStatic(controller.getStatic(), Serial, false)) return result;
		break;
	case 'a':
		if(result = parseAddressable(controller.getAddressable(), Serial, false)) return result;
		break;
	case 'o':
		controller.getStatic().on = 0;
		controller.getAddressable().on = 0;
		break;
	default:
		return -1;
	}

	return 0;
}

byte parseStatic(StaticConfig& config, Stream& stream, bool isInit) {
	DynamicJsonBuffer jsonBuffer(128);
	JsonObject& sttc = jsonBuffer.parseObject(stream);
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

	if(!isInit) {
		SD.remove("sttc");
		File configFile = SD.open("sttc", FILE_WRITE);
		if(!configFile) return ERR_S_CFG_SAVE_FAILURE;
		sttc.printTo(configFile);
		configFile.close();
	}
	
	return 0;
}

byte parseAddressable(AddressableConfig& config, Stream& stream, bool isInit) {
	char c = 0;
	int nRead = 0, size = 64;
	char* in = (char*)malloc(65);
	int retries = 0;
	while(c != ']') {
		if(!available(stream)) {
			free(in);
			return 35;
		}

		if(nRead >= size) {
			size = size + 64;
			in = (char*)realloc(in, size);
		}
		c = stream.read();
		in[nRead++] = c;
		retries = 0;
	}
	in[nRead] = 0;

	byte result = parseAddressableConfig(config, in, isInit);
	free(in);
	if(result != 0) return result;

	if(!isInit) return parseAddressableData(config);
	else return 0;
}

byte parseAddressableConfig(AddressableConfig& config, char* in, bool isInit) {
	DynamicJsonBuffer jsonBuffer(512);
	JsonArray& chans = jsonBuffer.parseArray(in);
	if(!chans.success()) return ERR_A_JSON_READ_FAILURE;
	config.clear();

	for(JsonObject& chan : chans) {
		if(!chan.containsKey("pin")) return ERR_A_MISSING_PIN;
		if(!chan.containsKey("speed")) return ERR_A_MISSING_SPEED;
		if(!chan.containsKey("ledCount")) return ERR_A_MISSING_LED_COUNT;
		if(!chan.containsKey("seqSize")) return ERR_A_MISSING_SEQ_SIZE;
		if(!pinIsValid(chan["pin"])) return ERR_A_BAD_PIN;
		Channel channel(chan["pin"], chan["speed"], chan["ledCount"], chan["seqSize"]);
		config.addChannel(channel);
	}

	if(!isInit) {
		SD.remove("addr");
		File configFile = SD.open("addr", FILE_WRITE);
		if(!configFile) return ERR_A_CFG_SAVE_FAILURE;
		chans.printTo(configFile);
		configFile.close();
	} else {
		config.on = 1;
	}

	return 0;
}

byte parseAddressableData(AddressableConfig& config) {
	for(int channelIndex = 0; channelIndex < config.channelCount; channelIndex++) {
		Channel& channel = config.getChannel(channelIndex);

		for(int cycleIndex = 0; cycleIndex < channel.seqSize; cycleIndex++) {
			char filename[9];
			int n = sprintf(filename, "%03d-%03d", channelIndex, cycleIndex);
			SD.remove(filename);
			File file = SD.open(filename, FILE_WRITE);
			if(!file) return ERR_A_FILE_OPEN_FAILURE;

			for(int ledIndex = 0; ledIndex < channel.ledCount; ledIndex++) {
				for(int colorIndex = 0; colorIndex < 3; colorIndex++) {
					if(!available(Serial)) {
						file.close();
						return 34;
					}
					file.write(Serial.read());
				}
			}
			SerialLong next = parseLong(Serial);
			for(int i = 0; i < 4; i++)
				file.write(next.bytes[i]);
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
			CRGB leds[channel.ledCount];

			char filename[9];
			sprintf(filename, "%03d-%03d", channelIndex, cycleIndex);
			File file = SD.open(filename, FILE_READ);
			if(!file) continue; // Do something else?

			for(int ledIndex = 0; ledIndex < channel.ledCount; ledIndex++) {
				for(int colorIndex = 0; colorIndex < 3; colorIndex++) {
					leds[ledIndex][colorIndex] = file.read();
				}
				//Serial.print(int(leds[ledIndex][0])); Serial.print(int(leds[ledIndex][1])); Serial.print(int(leds[ledIndex][2])); Serial.print('\n');
			}
			channel.next = parseLong(file).l;
			strip.setLeds(leds, channel.ledCount);
			strip.showLeds();
			file.close();
			//Serial.println(int(millis()-now));
		}
	}
}

SerialLong parseLong(Stream& stream) {
	SerialLong next;
	for(int i = 0; i < 4; i++) {
		if(!available(stream)) {
			next.l = 0;
			return next;
		}
		next.bytes[i] = stream.read();
	}
	return next;
}

bool available(Stream& stream) {
	for(int retries = 0; retries < 6; retries++) {
		if(stream.available()) return true;
		delay(200);
	}
	return false;
}

void respond(byte code) {
	Serial.write(code);
}
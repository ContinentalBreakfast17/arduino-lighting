#include <ArduinoJson.h>
#include <FastLED.h>
#include "lighting.hpp"

// function declarations
int parseCommand();
byte parseStaticConfig(StaticConfig& config);
byte parseAddressableConfig(AddressableConfig& config);
byte saveAddressableConfig(AddressableConfig& config);
void displayStatic(StaticConfig& config);
void displayAddressable(AddressableConfig& config);
void respond(byte code);

#define SERIAL_READ_TRIES 5
#define SERIAL_RETRY_WAIT 5


// Global controller which holds the current data
RGBController controller;

void setup () {
	Serial.begin(9600);
}

void loop() {
	byte parseResult = parseCommand();
	if(parseResult == 0 && !controller.on()) controller.start();
	respond(parseResult);

	displayStatic(controller.getStatic());
	displayAddressable(controller.getAddressable());
}

int parseCommand() {
	if(!Serial.available()) return -1;

	byte result = 0;
	byte code = Serial.read();
	if(code) {
		if(result = parseStaticConfig(controller.getStatic())) return result;
	} else {
		if(result = parseAddressableConfig(controller.getAddressable())) return result;
		if(result = saveAddressableConfig(controller.getAddressable())) return result;
	}

	return 0;
}

byte parseStaticConfig(StaticConfig& config, JsonObject& sttc) {
	DynamicJsonBuffer jsonBuffer(128);
	JsonObject& sttc = jsonBuffer.parseObject(Serial);
	if(!sttc.success()) return ERR_S_JSON_READ_FAILURE;

	byte result = 0;
	if(sttc.containsKey("mode") && (result = config.setMode(sttc["mode"]))) return ERR_S_BAD_MODE;
	if(sttc.containsKey("speed") && (result = config.setSpeed(sttc["speed"]))) return ERR_S_BAD_SPEED;
	if(sttc.containsKey("pins")) {
		byte pins[3];
		if(sttc.get<JsonArray&>("pins").copyTo(pins) != 3) return ERR_S_BAD_PIN_LENGTH;
		if(result = config.setPins(pins)) return ERR_S_BAD_PINS;
		for(byte i = 0; i < 3; i++) pinMode(pins[i], OUTPUT);
	}
	if(sttc.containsKey("color")) {
		byte color[3];
		if(sttc.get<JsonArray&>("color").copyTo(color) != 3) return ERR_S_BAD_COLOR_LENGTH;
		if(result = config.setColor(color)) return ERR_S_BAD_COLOR;
	}
	return 0;
}

byte parseAddressableConfig(AddressableConfig& config) {
	DynamicJsonBuffer jsonBuffer(512);
	JsonArray& chans = jsonBuffer.parseArray(Serial);
	if(!chans.success()) return ERR_S_JSON_READ_FAILURE;

	vector<Channel> channels;
	for(JsonObject& chan : chans) {
		if(!chan.containsKey("pin")) return ERR_A_MISSING_PIN;
		if(!chan.containsKey("speed")) return ERR_A_MISSING_SPEED;
		if(!chan.containsKey("ledCount")) return ERR_A_MISSING_LED_COUNT;
		if(!chan.containsKey("seqSize")) return ERR_A_MISSING_SEQ_SIZE;
		Channel channel(chan["pin"], chan["speed"], chan["ledCount"], chan["seqSize"]);
		channels.push_back(channel);
	}
	config.setChannels(channels);
	return 0;
}

byte saveAddressableConfig(AddressableConfig& config) {
	vector<Channel>& channels = config.getChannels();
	for(int channelIndex = 0; channelIndex < channels.size(); channelIndex++) {
		Channel& channel = channels.at(channelIndex);

		for(int cycleIndex = 0; cycleIndex < channel.seqSize; i++) {
			byte filename[128];
			sprintf(filename, "channel%d.cycle%d.json", channelIndex, cycleIndex);
			SD.remove(filename);
			File file = SD.open(filename, FILE_WRITE);
			if(!file) return ERR_A_FILE_OPEN_FAILURE;

			byte serialReadTries = 0, c = 0;
			while(c != '\r') {
				if(!Serial.available()) {
					if(serialReadTries > SERIAL_READ_TRIES) return ERR_A_SERIAL_TIMEOUT;
					serialReadTries++;
					delay(SERIAL_RETRY_WAIT);
				}

				c = Serial.read();
				if(c != '\r' && file.write(c) != 1) return ERR_A_FILE_WRITE_FAILURE;
			}
			file.close();
		}
	}
}

void displayStatic(StaticConfig& config) {
	if(!controller.on()) return;
	long long now = millis();
	if(config.shouldDisplay(now)) {
		byte* color = config.getDisplayColor();
		byte* pins = config.getPins();
		for(byte i = 0; i < 3; i++) analogWrite(pins[i], color[i]);
	}
}

void displayAddressable(AddressableConfig& config) {
	if(!controller.on()) return;

	vector<Channel>& channels = config.getChannels();
	for(byte channelIndex = 0; channelIndex < channels.size(); channelIndex++) {
		Channel& channel = channels.at(channelIndex);
		long long now = millis();

		if(channel.shouldDisplay(now)) {
			byte pin = channel.getPin();
			int cycleIndex = channel.getIndex();

			byte filename[128];
			sprintf(filename, "channel%d.cycle%d.json", channelIndex, cycleIndex);
			File file = SD.open(filename, FILE_READ);
			if(!file) continue; // Do something else?

			DynamicJsonBuffer jsonBuffer(1300);
			JsonArray& strip = jsonBuffer.parseArray(file);
			if(!strip.success()) continue; // Do something else?

			for(int ledIndex = 0; ledIndex < strip.size(); ledIndex++) {
				for(JsonArray& color : strip.get<JsonArray&>(ledIndex)) {
					Serial.print(color[0]); Serial.print(color[1]); Serial.print(color[2]); Serial.print('\n');
				}
			}
		}
	}
}

void respond(byte code) {
	Serial.print(code);
}
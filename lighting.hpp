#include <FastLED.h>

using namespace std;

// RGB modes
#define STATIC_MODE  0
#define RAINBOW_MODE 1
#define FADE_MODE    2
#define STROBE_MODE  4

typedef unsigned char byte;

class StaticConfig {
private:
	byte pins[3], color[3], rainbow[3], display[3];
	byte mode, rainbowIndex, strobe, fade, fadeDir;
	long speed;
	long long lastDisplay;
	void doRainbow() {
		rainbow[rainbowIndex]--;
		rainbow[(rainbowIndex == 2) ? (0) : (rainbowIndex + 1)]++;
		if(rainbow[rainbowIndex] == 0) {
			rainbowIndex = (rainbowIndex == 2) ? (0) : (rainbowIndex + 1);
		}
		display[0] = rainbow[0]; display[1] = rainbow[1]; display[2] = rainbow[2];
	};
	void doStrobe() {
		strobe = !strobe;
		display[0] = color[0]*(strobe); display[1] = color[1]*(strobe); display[2] = color[2]*(strobe);
	};
	void doFade() {
		fade += fadeDir-2;
		if(fade == 255) fadeDir = 1;
		else if(fade == 0) fadeDir = 3;
		float f = float(fade);
		display[0] = color[0]*(f/255); display[1] = color[1]*(f/255); display[2] = color[2]*(f/255);
	};

public:
	byte on;
	StaticConfig() {
		// start rainbow all red, fading to green
		rainbow[0] = 255; rainbow[1] = 0; rainbow[2] = 0;
		rainbowIndex = 0;
		// start strobe and fade off
		strobe = 0;
		fade = 0;
		fadeDir = 3;
		// default speed of 5 ms
		speed = 5;
		// init lastDisplay to 0 so that it updates on 1st loop
		lastDisplay = 0;
	};
	byte* getPins() {
		return pins;
	};
	byte* getDisplayColor(){
		switch(mode) {
			case STATIC_MODE:
				display[0] = color[0]; display[1] = color[1]; display[2] = color[2];
				break;
			case RAINBOW_MODE:
				doRainbow();
				break;
			case FADE_MODE:
				doFade();
				break;
			case STROBE_MODE:
				doStrobe();
				break;
		}
		return display;
	};
	byte shouldDisplay(long long now) {
		if(now < lastDisplay + speed) return 0;
		lastDisplay = now;
		return 1;
	};
	byte setPins(byte pins[3]) {
		for(byte i = 0; i < 3; i++) {
			if(pins[i] < 0) return 1;
			this->pins[i] = pins[i];
		}
		return 0;
	};
	byte setColor(byte color[3]) {
		for(byte i = 0; i < 3; i++) {
			this->color[i] = color[i];
		}
		return 0;
	};
	byte setColorChannel(byte channel, byte value) {
		if(channel < 0 || channel > 2) return 1;
		color[channel] = value;
		return 0;
	};
	byte setMode(byte mode) {
		switch(mode) {
			case STATIC_MODE: 
			case RAINBOW_MODE: 
			case FADE_MODE: 
			case STROBE_MODE:
				this->mode = mode;
				return 0;
			default:
				return 1;
		}
	};
	byte setSpeed(long speed) {
		if(speed < 0) return 1;
		this->speed = speed;
		return 0;
	};
};

class Channel {
private:
	int seqIndex;
	long speed;
	long long lastDisplay;
public:
	byte pin, seqSize;
	long ledCount;
	CRGB* leds;

	Channel(byte pin, long speed, long ledCount, byte seqSize) {
		this->pin = pin;
		this->speed = speed;
		this->ledCount = ledCount;
		this->seqSize = seqSize;
		leds = (CRGB*)malloc(sizeof(CRGB)*ledCount);
		seqIndex = 0;
		lastDisplay = 0;
	};
	byte shouldDisplay(long long now) {
		if(now < lastDisplay + speed) return 0;
		lastDisplay = now;
		return 1;
	};
	byte getIndex() {
		if(seqIndex >= seqSize) seqIndex = 0;
		return seqIndex++;
	};
	void clear() {
		delete(leds);
	};
};

class AddressableConfig {
public:
	Channel* channels;
	byte     channelCount, on;

	AddressableConfig() {
		channelCount = 0;
	};
	byte addChannel(Channel channel) {
		channels = (Channel*)realloc(channels, (channelCount+1)*sizeof(Channel));
		if(channels == NULL) return 1;
		channels[channelCount++] = channel;
	};
	Channel& getChannel(int index) {
		return channels[index];
	};
	void clear() {
		for(int i = 0; i < channelCount; i++) {
			channels[i].clear();
		}
		delete(channels);
		channelCount = 0;
		on = 0;
	};
};

/*
	Controller containing static and addressable configurations
*/
class RGBController {
private:
	StaticConfig      staticConfig;
	AddressableConfig addressableConfig;
	bool              running;

public:
	RGBController() {
		// set to off
		running = false;
	};
	StaticConfig& getStatic() {
		return staticConfig;
	};
	AddressableConfig& getAddressable() {
		return addressableConfig;
	};
};

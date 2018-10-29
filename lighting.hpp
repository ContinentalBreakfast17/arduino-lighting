#include <vector>

using namespace std;

// RGB modes
#define STATIC  0
#define RAINBOW 1
#define FADE    2
#define STROBE  4

typedef unsigned int uint;
class RGB {
public:
	uint color[3];
	RGB(uint color[3]) {
		this->color[0] = color[0]; this->color[1] = color[1]; this->color[2] = color[2];
	};
	uint operator[](int i) {
		return color[i];
	}
};
typedef vector<RGB> RGBStrip;
typedef vector<RGBStrip> RGBStripSeq;


class StaticConfig {
private:
	int pins[3];
	uint color[3], rainbow[3], display[3];
	int  mode, speed, rainbowIndex, strobe, fade, fadeDir;
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
		fade += fadeDir;
		if(fade >= 255) fadeDir = -1;
		else if(fade <= 0) fadeDir = 1;
		display[0] = color[0]*(fade/255); display[1] = color[1]*(fade/255); display[2] = color[2]*(fade/255);
	};

public:
	StaticConfig() {
		// start rainbow all red, fading to green
		rainbow[0] = 255; rainbow[1] = 0; rainbow[2] = 0;
		rainbowIndex = 0;
		// start strobe and fade off
		strobe = 0;
		fade = -1;
		fadeDir = 1;
		// default speed of 5 ms
		speed = 5;
		// init lastDisplay to 0 so that it updates on 1st loop
		lastDisplay = 0;
	};
	int* getPins() {
		return pins;
	};
	uint* getDisplayColor(){
		switch(mode) {
			case STATIC:
				display[0] = color[0]; display[1] = color[1]; display[2] = color[2];
				break;
			case RAINBOW:
				doRainbow();
				break;
			case FADE:
				doFade();
				break;
			case STROBE:
				doStrobe();
				break;
		}
		return display;
	};
	bool shouldDisplay(long long now) {
		if(now < lastDisplay + speed) return false;
		lastDisplay = now;
		return true;
	};
	int setPins(int pins[3]) {
		int i;
		for(i = 0; i < 3; i++) {
			if(pins[i] < 0) return 1;
			this->pins[i] = pins[i];
		}
		return 0;
	};
	int setColor(RGB color) {
		int i;
		for(i = 0; i < 3; i++) {
			this->color[i] = color[i];
		}
		return 0;
	};
	int setColorChannel(int channel, uint value) {
		if(channel < 0 || channel > 2) return 1;
		color[channel] = value;
		return 0;
	};
	int setMode(int mode) {
		switch(mode) {
			case STATIC: 
			case RAINBOW: 
			case FADE: 
			case STROBE:
				this->mode = mode;
				return 0;
			default:
				return 1;
		}
	};
	int setSpeed(int speed) {
		if(speed < 0 || speed > 60*1000) return 1;
		this->speed = speed;
		return 0;
	};
};

class Channel {
private:
	RGBStripSeq sequence;
	int         pin, speed, sequenceIndex;
	long long   lastDisplay;

public:
	Channel() {
		sequenceIndex = 0;
		speed = 5;
		lastDisplay = 0;
	};
	int getPin() {
		return pin;
	};
	RGBStrip getDisplayStrip() {
		if(sequenceIndex >= sequence.size()) sequenceIndex = 0;
		return sequence.at(sequenceIndex++);
	};
	bool shouldDisplay(long long now) {
		if(now < lastDisplay + speed) return false;
		lastDisplay = now;
		return true;
	};
	int setSequence(RGBStripSeq sequence) {
		this->sequence.swap(sequence);
	};
	int setPin(int pin) {
		if(pin < 0) return 1;
		this->pin = pin;
		return 0;
	};
	int setSpeed(int speed) {
		if(speed < 0 || speed > 60*1000) return 1;
		this->speed = speed;
		return 0;
	};
};

class AddressableConfig {
private:
	vector<Channel> channels;

public:
	AddressableConfig() {

	};
	vector<Channel>& getChannels() {
		return channels;
	};
	int setChannels(vector<Channel> channels) {
		this->channels.swap(channels);
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
	void start() {
		running = true;
	};
	bool on() {
		return running;
	};
};

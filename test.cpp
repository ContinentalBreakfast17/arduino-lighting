/*#include "lighting.hpp"
#include <iostream>
#include <vector>

void addChannels(AddressableConfig& cfg) {
	RGBStripSeq sequence;
	for(uint i = 0; i < 3; i++) {
		uint led[3] = {0,0,0}; led[i] = 255; RGB rgb1(led);
		uint led2[3] = {15*(i+1), 30*(i+1), 60*(i+1)}; RGB rgb2(led2);
		RGBStrip strip;
		strip.push_back(led);
		strip.push_back(led2);
		sequence.push_back(strip);
	}

	Channel channel;
	channel.setSequence(sequence);
	vector<Channel> channels;
	channels.push_back(channel);

	cfg.setChannels(channels);
}

int main() {
	RGBController ctrl;
	AddressableConfig& cfg = ctrl.getAddressable();

	addChannels(cfg);
	

	for(int i = 0; i < 9; i++) {
		vector<Channel>& chans = cfg.getChannels();
		for(int c = 0; c < chans.size(); c++) {
			Channel& chan = chans.at(c);
			RGBStrip strip = chan.getDisplayStrip();
			cout << "-- Iteration " << i << " --\n";
			for(int j = 0; j < strip.size(); j++) {
				RGB led = strip.at(j);
				printf("(%d,%d,%d) ", led[0], led[1], led[2]);
			}
			cout << "\n\n";
		}
		
	}
{"code":1,"staticConfig":{"mode":0,"pins":[9,10,11],"color":[100,0,200]}}
{"code":1,"staticConfig":{"mode":1}}
}*/
#include <ArduinoJson.h>

#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>

//#include <FastLED.h>
#include "lighting.hpp"

#define STATIC_CMD      1
#define ADDRESSABLE_CMD 2

typedef struct SerialCommand {
  int       code;
} Command;

typedef struct SerialResponse {
  const char* message;
  int         code;
} Response;

// function declarations
int parseStaticConfig(StaticConfig& config, JsonObject& msg);
int parseCommand(Command* cmd);
int parseAddressableConfig(AddressableConfig& config, JsonArray& msg);
void displayStatic(StaticConfig& config);
void displayAddressable(AddressableConfig& config);


// Global controller which holds the current data
RGBController controller;

void setup () {
  Serial.begin(9600);
}

void loop() {
  Command cmd;
  int parseResult = parseCommand(&cmd);
  if(parseResult == 0 && !controller.on()) controller.start();
  else if(parseResult > 0) Serial.print(parseResult);

  displayStatic(controller.getStatic());
  displayAddressable(controller.getAddressable());
}

int parseCommand(Command* cmd) {
  if(!Serial.available()) return -1;

  DynamicJsonBuffer jsonBuffer(1024);
  JsonObject& msg = jsonBuffer.parseObject(Serial);
  if(!msg.success()) return 3;
  cmd->code = msg["code"];

  int result = 0;
  if(cmd->code & STATIC_CMD) {
    result = result | parseStaticConfig(controller.getStatic(), msg["staticConfig"]);
  }
  if(cmd->code & ADDRESSABLE_CMD) {
    result = result | parseAddressableConfig(controller.getAddressable(), msg["addressableConfig"]);
  }

  return result;
}

int parseStaticConfig(StaticConfig& config, JsonObject& obj) {
  int result = 0;
  if(obj.containsKey("mode") && (result = config.setMode(obj["mode"]))) return result;
  if(obj.containsKey("speed") && (result = config.setSpeed(obj["speed"]))) return result;
  if(obj.containsKey("pins")) {
    int pins[3];
    if(obj.get<JsonArray&>("pins").copyTo(pins) != 3) return 1;
    if(result = config.setPins(pins)) return result;
    for(int i = 0; i < 3; i++) pinMode(pins[i], OUTPUT);
  }
  if(obj.containsKey("color")) {
    unsigned int color[3];
    if(obj.get<JsonArray&>("color").copyTo(color) != 3) return 1;
    if(result = config.setColor(color)) return result;
  }
  return 0;
}

int parseAddressableConfig(AddressableConfig& config, JsonArray& arr) {
  int result = 0;
  vector<Channel> channels;

  for(JsonObject& chan : arr) {
    RGBStripSeq sequence;
    Channel channel;

    if(!chan.containsKey("sequence")) return 1;
    if(chan.containsKey("speed") && (result = channel.setSpeed(chan["speed"]))) return result;

    if(!chan.containsKey("pin")) return 1;
    int pin = chan["pin"];
    if(result = channel.setPin(pin)) return result;
    
    for(JsonArray& strip : chan.get<JsonArray&>("sequence")) {
      RGBStrip rgbStrip;

      for(JsonArray& led : strip) {
        unsigned int color[3];
        led.copyTo(color);
        rgbStrip.push_back(color);
      }

      sequence.push_back(rgbStrip);
    }

    channel.setSequence(sequence);
    channels.push_back(channel);
  }

  config.setChannels(channels);
}

void displayStatic(StaticConfig& config) {
  if(!controller.on()) return;
  long long now = millis();
  if(config.shouldDisplay(now)) {
    unsigned int* color = config.getDisplayColor();
    int* pins = config.getPins();
    for(int i = 0; i < 3; i++) analogWrite(pins[i], color[i]);
  }
}

void displayAddressable(AddressableConfig& config) {
  if(!controller.on()) return;

  vector<Channel>& chans = config.getChannels();
  for(int i = 0; i < chans.size(); i++) {
    Channel& chan = chans.at(i);
    int pin = chan.getPin();
    long long now = millis();

    if(chan.shouldDisplay(now)) {
      RGBStrip strip = chan.getDisplayStrip();
      for(int j = 0; j < strip.size(); j++) {
        RGB color = strip.at(j);
        //for(int k = 0; k < 3; k++) analogWrite(pin, color[k]);
      }
    }
  }
}

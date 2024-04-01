#include <Arduino.h>
#include <ArduinoBLE.h>
#include <camera.h>
#include <parser.h>
#include <commands.h>

#define COMMAND_LINE_WIDTH 128
char commandLineBuffer[COMMAND_LINE_WIDTH];
uint32_t lastAlive;
bool ledStatus;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(7500);
  setupCamera(0);
  setupCommands();
}

void loop() {
  if(Serial.available() > 0){
    memset(&commandLineBuffer, 0, COMMAND_LINE_WIDTH);
    Serial.readBytesUntil('\r', commandLineBuffer, COMMAND_LINE_WIDTH);
    if(Serial.peek() == '\n'){
      Serial.read();
    }

    argx_type argx = parseArgx(commandLineBuffer, COMMAND_LINE_WIDTH, true);
    executeCommands(argx.argc, argx.argv);
  }
  
  if(millis() > lastAlive + 1000){
    digitalWrite(LED_BUILTIN, ledStatus);
    ledStatus = !ledStatus;
    lastAlive = millis();
  }
}

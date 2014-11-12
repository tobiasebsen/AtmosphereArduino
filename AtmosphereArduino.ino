/*
 * ATMOSPHERE - Version 0.3
 * by Alexandra Instituttet A/S
 *
 * Author: Tobias Ebsen
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 */

#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>   // https://github.com/media-architecture/WIZ_Ethernet_Library
#include <ArtNet.h>     // https://github.com/media-architecture/Arduino_ArtNet/
#include <TrueRandom.h> // https://code.google.com/p/tinkerit/wiki/TrueRandom
#include <EthernetUdp.h>
#include <OSCMessage.h>
#include <FastLED.h>    // https://github.com/FastLED/FastLED

#define LED_PIN  9

// Firmware version
#define FIRMWARE_VERSION_HIGH 0
#define FIRMWARE_VERSION_LOW  1

// Default Configuration
ArtNetConfig config = {
  {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, // MAC
  {172, 16, 0, 1},                     // IP
  {255, 255, 255, 0},                   // Subnet mask
  0x1936,                               // UDP port
  false,                                 // DHCP
  0, 0,                                 // Net and subnet
  "Atmosphere",                      // Short name
  "Atmosphere by Alexandra Instituttet A/S", // Long name
  2,                                    // Number of ports
  {ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,ARTNET_TYPE_DMX|ARTNET_TYPE_OUTPUT,0,0}, // Port types
  {0, 1, 0, 0},                         // Input port addresses
  {0, 1, 0, 0}                          // Output port addresses
};

ArtNet artnet = ArtNet(config, 600);

// LED control
static WS2811Controller800Khz<5, GRB> port1;
static WS2811Controller800Khz<6, GRB> port2;
static WS2811Controller800Khz<8, GRB> port3;
static WS2811Controller800Khz<7, GRB> port4;

IPAddress outIp(172,16,0,100);
const unsigned int outPort = 9999;
EthernetUDP osc;

void setup() {
  
  MCUSR=0;
  wdt_disable();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  // Initialization and optimization
  ethernetInit();
  
  // Read config from EEPROM
  config.mac[5] = config.ip[3];
  configGet(config);
  config.verHi = FIRMWARE_VERSION_HIGH;
  config.verLo = FIRMWARE_VERSION_LOW;
  
  // Initialize FastLED
  port1.init();
  port2.init();
  port3.init();
  port4.init();

  // Start ArtNet
  artnet.begin();
  osc.begin(8888);
  
  ethernetMaximize();
  
  //pinMode(1, OUTPUT);
  //digitalWrite(1, LOW);

  pinMode(2, INPUT);
  attachInterrupt(0, pulse, CHANGE);

  wdt_enable(WDTO_8S);
  digitalWrite(LED_PIN, LOW);
}

unsigned long time;
volatile unsigned long rise;
volatile unsigned long pulseWidth;
volatile boolean isnew;

void loop() {
  
  int r = artnet.parsePacket();
  if(r > 0) {

    digitalWrite(LED_PIN, HIGH);
    
    switch(artnet.getOpCode()) {
      
      case ARTNET_OPCODE_DMX: {
        //noInterrupts();
          int len = artnet.getDmxLength();
          byte *data = artnet.getDmxData();
          switch(artnet.getDmxPort()) {
            case 0: {
              port1.show((CRGB*)data, 60);
              port2.show((CRGB*)data+60, 60);
            } break;
            case 1: {
              port3.show((CRGB*)data, 60);
              port4.show((CRGB*)data+60, 60);

              /*unsigned long t = millis();
              if (t - time >= 50) {
                digitalWrite(1, HIGH);
                delayMicroseconds(20);
                digitalWrite(1, LOW);
                time = t;
              }*/
            } break;
          }
          //interrupts();
      } break;
      
      case ARTNET_OPCODE_IPPROG:
        artnet.handleIpProg();
        if (artnet.getIpCommand() & ARTNET_IPCMD_PROGRAM)
          configWrite(config);
        break;
        
      case ARTNET_OPCODE_ADDRESS:
        artnet.handleAddress();
        configWrite(config);
        break;
      
      default:
        artnet.handleAny();
    }
    digitalWrite(LED_PIN, LOW);
  }
  else {

    if (isnew) {
      OSCMessage msg("/distance");
      msg.add((int32_t)config.portAddrOut[0]);
      msg.add((int32_t)pulseWidth);
  
      osc.beginPacket(outIp, outPort);
      msg.send(osc);
      osc.endPacket();
      msg.empty();
      isnew = false;
    }
  }
 
  // Check DHCP lease
  //if(config.dhcp)
  //  artnet.maintain();

  wdt_reset();
}

void pulse() {
  if (digitalRead(2) == HIGH)
    rise = micros();
  else {
    pulseWidth = micros() - rise;
    isnew = true;
  }
}

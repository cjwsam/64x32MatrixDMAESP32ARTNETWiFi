
#ifndef ARTNET_WIFI_H
#define ARTNET_WIFI_H

#include <functional>
#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_SAMD)
#if defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#else
#include <WiFiNINA.h>
#endif
#else
#error "Architecture not supported!"
#endif
#include "AsyncUDPCLEAN.h"

// UDP specific
#define ART_NET_PORT 6454
// Opcodes
#define ART_POLL 0x2000
#define ART_DMX 0x5000
// Buffers
#define MAX_BUFFER_ARTNET 530
// Packet
#define ART_NET_ID "Art-Net"
#define ART_DMX_START 18

#define DMX_FUNC_PARAM uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data
typedef std::function <void (DMX_FUNC_PARAM)> StdFuncDmx_t;

class ArtnetWifi
{
public:
  ArtnetWifi();

  void begin(String hostname = "");
  uint16_t read(void);
  /* returns 1 for Ok, or 0 on problem */
  int write(void);
  int write(IPAddress ip);
  void setByte(uint16_t pos, uint8_t value);
  void printPacketHeader(void);
  void printPacketContent(void);

  // Return a pointer to the start of the DMX data
  inline uint8_t* getDmxFrame(void)
  {
    return artnetPacket + ART_DMX_START;
  }

  inline uint16_t getOpcode(void)
  {
    return opcode;
  }

  inline uint8_t getSequence(void)
  {
    return sequence;
  }

  inline uint16_t getUniverse(void)
  {
    return incomingUniverse;
  }

  inline void setUniverse(uint16_t universe)
  {
    outgoingUniverse = universe;
  }

  inline void setPhysical(uint8_t port)
  {
    physical = port;
  }

  [[deprecated]]
  inline void setPhisical(uint8_t port)
  {
    setPhysical(port);
  }

  inline uint16_t getLength(void)
  {
    return dmxDataLength;
  }

  inline void setLength(uint16_t len)
  {
    dmxDataLength = len;
  }

  inline void setArtDmxCallback(void (*fptr)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data))
  {
    artDmxCallback = fptr;
  }

  inline void setArtDmxFunc(StdFuncDmx_t func)
  {
    artDmxFunc = func;
  }

private:
  uint16_t makePacket(void);

  AsyncUDP Udp;
  String host;
  uint8_t artnetPacket[MAX_BUFFER_ARTNET];
  uint16_t packetSize;
  uint16_t opcode;
  uint8_t sequence;
  uint8_t physical;
  uint16_t incomingUniverse;
  uint16_t outgoingUniverse;
  uint16_t dmxDataLength;
  void (*artDmxCallback)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);
  StdFuncDmx_t artDmxFunc;
  static const char artnetId[];
};

#endif

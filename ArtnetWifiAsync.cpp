
#include "ArtnetWifiAsync.h"


const char ArtnetWifi::artnetId[] = ART_NET_ID;

ArtnetWifi::ArtnetWifi() {}

void ArtnetWifi::begin(String hostname)
{

  while(!Udp.listen(ART_NET_PORT))
  {
    Serial.println("Not Connected UDP");
    yield(); //////////////////////////////////////////////////////////////////////////////////
  }
      Serial.println("Connected UDP");

  host = hostname;
  sequence = 1;
  physical = 0;
}

uint16_t ArtnetWifi::read(void)
{
  
  Udp.onPacket([&](AsyncUDPPacket packet)
 {
if(packet.isBroadcast())
          {
          
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", universe: ");
            Serial.print(packet.data()[14] | packet.data()[15] << 8);
            Serial.print(", SEQ: ");
            Serial.print( packet.data()[12]);
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length());

        }
        
  packetSize = packet.length();

  if (packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
  {
      packet.read(packet.data(), MAX_BUFFER_ARTNET);

      // Check that packetID is "Art-Net" else ignore
      if (memcmp(packet.data(), artnetId, sizeof(artnetId)) != 0) {
        return 0;
      }

    uint16_t  opcode = packet.data()[8] | packet.data()[9] << 8;

      if (opcode == ART_DMX)
      {
        sequence = packet.data()[12];
        incomingUniverse = packet.data()[14] | packet.data()[15] << 8;
        dmxDataLength = packet.data()[17] | packet.data()[16] << 8;

        if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, packet.data() + ART_DMX_START);
        if (artDmxFunc) {
          artDmxFunc(incomingUniverse, dmxDataLength, sequence, packet.data() + ART_DMX_START);
        }
        return ART_DMX;
      }
      if (opcode == ART_POLL)
      {
        return ART_POLL;
      } 
  }
         
  return 0;

 });
}

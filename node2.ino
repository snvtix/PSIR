#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
ZsutIPAddress ip(192, 168, 56, 121);
ZsutIPAddress servIp(192, 168, 56, 119);

unsigned int localPort = 1234;
unsigned int servPort = 1234;     

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
char hello[] = "001101"; 
char z0;
char z1;
char z2;
char win[] = "101101";
int packetSize;

ZsutEthernetUDP Udp;

void setup() {  
  ZsutEthernet.begin(mac, ip);
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println(ZsutDigitalRead());
  //ustawienie wzorca
  if(ZsutDigitalRead() == 0){
    z0 = '0';
    z1 = '0';
    z2 = '0';
  }else if(ZsutDigitalRead() == 1){
    z0 = '1';
    z1 = '0';
    z2 = '0';
  }else if(ZsutDigitalRead() == 2){
    z0 = '0';
    z1 = '1';
    z2 = '0';
  }else if(ZsutDigitalRead() == 3){
    z0 = '1';
    z1 = '1';
    z2 = '0';
  }else if(ZsutDigitalRead() == 4){
    z0 = '0';
    z1 = '0';
    z2 = '1';
  }else if(ZsutDigitalRead() == 5){
    z0 = '1';
    z1 = '0';
    z2 = '1';
  }else if(ZsutDigitalRead() == 6){
    z0 = '0';
    z1 = '1';
    z2 = '1';
  }else if(ZsutDigitalRead() == 7){
    z0 = '1';
    z1 = '1';
    z2 = '1';
  }
  //wiadomosc hello
  Udp.begin(localPort);
  packetSize = Udp.parsePacket();
  Udp.beginPacket(servIp, servPort);
  Udp.write(hello);
  Udp.endPacket();
}

void loop() {
  packetSize = Udp.parsePacket();
  Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  if(packetBuffer[0] == '0' && packetBuffer[1] == '1' && packetBuffer[5] == '1' && packetBuffer[6] == '0' && packetBuffer[7] == '1'){
    Serial.println(packetBuffer);
    if(packetBuffer[2] == z2 && packetBuffer[3] == z1 && packetBuffer[4] == z0){
      delay(100);
      //wiadomosc win
      packetSize = Udp.parsePacket();
      Udp.beginPacket(servIp, servPort);
      Udp.write(win);
      Udp.endPacket();
    }
  }
} 
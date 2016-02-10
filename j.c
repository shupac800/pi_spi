
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>

// function declarations
unsigned char initializePi();
int sendcmd0();
int getResponse();
void sendByteArr(unsigned char *byteArr, int length);
unsigned char RWBit(unsigned char bit);
void delayLoop(int delay;);

// globals
char MOSI_pin = 12;
char MISO_pin = 13;
char SCLK_pin = 14;
char CE0_pin = 10;

void main()
{
  unsigned char byteArr[6];
  unsigned char cmd0[6] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
  unsigned char cmd8[6] = { 0x48, 0x00, 0x00, 0x01, 0xAA, 0x0F };
  unsigned char cmd58[6] = {0x7A,0x00,0x00,0x00,0x00,0x75};

  initializePi();

  // send CMD0
  memcpy(byteArr,cmd0,6);
  sendByteArr(byteArr,6);
  printf("CMD0 generates response %02X\n",getResponse());

  // send CMD8
  memcpy(byteArr,cmd8,6);
  sendByteArr(byteArr,6);
  printf("CMD8 generates response %02X\n",getResponse());

  // send CMD58
  memcpy(byteArr,cmd58,6);
  sendByteArr(byteArr,6);
  printf("CMD58 generates response %02X\n",getResponse());

  // rest state
  digitalWrite(MOSI_pin,1);
  digitalWrite(CE0_pin,1);
  digitalWrite(SCLK_pin,1);
  return;
}

void sendByteArr(unsigned char *byteArr, int length) {
  int i, j;
  unsigned char MOSI_bit, MISO_bit, MISO_byte;

  for (j = 0; j < length; j++) {

//    printf("Sending byte %02X\n",byteArr[j]);
    MISO_byte = 0;

    for (i = 7; i >= 0 ; i--) {
//    printf("in loop now at i = %d\n",i);
      MOSI_bit = (byteArr[j] >> i) & 1;
//    printf("  bit %d is %d\n",7,MOSI_bit);
      MISO_bit = RWBit(MOSI_bit);
      MISO_byte = (MISO_byte << 1) + MISO_bit;
    }
//    printf("  Received byte %02X\n",MISO_byte);
  }

  digitalWrite(MOSI_pin,1); // MOSI is held high when not sending
  digitalWrite(CE0_pin,0); // make sure CE is LOW

  return;
}

unsigned char initializePi()
{
  wiringPiSetup();
  pinMode(MOSI_pin,OUTPUT);
  pinMode(MISO_pin,INPUT);
  pinMode(CE0_pin,OUTPUT);
  pinMode(SCLK_pin,OUTPUT);

  digitalWrite(SCLK_pin,0);  // initialize clock as LOW
  digitalWrite(MOSI_pin,1);  // MOSI is HIGH when no message is being sent
  digitalWrite(CE0_pin,1);   // chip select is HIGH during initialization

  // place card in SPI mode
  int i;
  for (i = 0; i < 74; i++) {
    digitalWrite(SCLK_pin,1);
    digitalWrite(SCLK_pin,0);
  }

  digitalWrite(CE0_pin,0);

  return 0;
}

int sendcmd0()
{
}

int getResponse()
{
  int i, j;
  unsigned char MISO_byte;

  // wait for response
  for (i = 0; i < 16; i++) {  // wait 16 clock cycles
//  printf("Toggling clock... i=%d\n",i);
    digitalWrite(SCLK_pin,1);
    digitalWrite(SCLK_pin,0);
    char MISO_bit = digitalRead(MISO_pin);
    if (!MISO_bit) {  // return message commences
      for (j = 0; j < 8; j++) {  // next 8 bits are response
//      printf("Got response bit %d\n",MISO_bit);
        MISO_byte = (MISO_byte << 1) + MISO_bit;
        digitalWrite(SCLK_pin,1);
        digitalWrite(SCLK_pin,0);
        MISO_bit = digitalRead(MISO_pin);
      }
      return MISO_byte;
    }
  } 
  return -1; // no response received
}

unsigned char RWBit(unsigned char MOSI_bit) {
  // assumes we enter with SCLK = LOW
  
  digitalWrite(MOSI_pin,MOSI_bit);  // write the bit
  digitalWrite(SCLK_pin,1);  // pull SCLK high
  char MISO_bit = digitalRead(MISO_pin); // latch bit from MISO
  digitalWrite(SCLK_pin,0);  // pull SCLK low (shift MOSI and MISO registers)

  return MISO_bit;

}

void delayLoop(int delay) {
  int i;

  for (i = 0; i < delay; i++) {
    //__asm__("nop");
    i = i;
  }
  return;
}


#include <stdio.h>
#include <string.h>
#include <wiringPi.h>

// function declarations
unsigned char initializePi();
int getResponse();
void sendByteArr(unsigned char *byteArr, int length);
unsigned char RWByte(unsigned char byte);
void readBlock();

// globals
char MOSI_pin = 12;
char MISO_pin = 13;
char SCLK_pin = 14;
char CE0_pin = 10;
unsigned char readBuffer[512];

void main()
{
  unsigned char byteArr[6];
  unsigned char cmd0[6] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
  unsigned char cmd1[6] = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xF9 };
  unsigned char cmd8[6] = { 0x48, 0x00, 0x00, 0x01, 0xAA, 0x0F };
  unsigned char cmd16[6] = { 0x50, 0x00, 0x02, 0x00, 0x00, 0xFF };
  unsigned char cmd18[6] = { 0x52, 0x00, 0x10, 0x00, 0x00, 0x5B };
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

  // send CMD16, set block length to 512 bytes
  memcpy(byteArr,cmd16,6);
  sendByteArr(byteArr,6);
  printf("CMD16 generates response %02X\n",getResponse());

  // send CMD18, read block
  memcpy(byteArr,cmd18,6);
  sendByteArr(byteArr,6);
  printf("CMD18 generates response %02X\n",getResponse());
  readBlock(); 

  return;
}

void sendByteArr(unsigned char *byteArr, int length) {
  int i, j;
  unsigned char MOSI_bit, MISO_bit, MISO_byte;

  for (j = 0; j < length; j++) {
    RWByte(byteArr[j]);
  }

  // end SPI transaction
  digitalWrite(MOSI_pin,1);
  digitalWrite(CE0_pin,1);
  digitalWrite(SCLK_pin,1);
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

int getResponse()
{
  int i, j;
  unsigned char MISO_byte;

  // wait for response
  int n = 16; // wait n clock cycles
  for (i = 0; i < n; i++) {
    digitalWrite(SCLK_pin,1);
    digitalWrite(SCLK_pin,0);
    char MISO_bit = digitalRead(MISO_pin);
    if (!MISO_bit) {  // return message commences
      for (j = 0; j < 8; j++) {  // next 8 bits are response
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

unsigned char RWByte(unsigned char byte) {
  // assumes we enter with SCLK = LOW

  int i;
  unsigned char MOSI_bit, MISO_bit, byteRead=0;

  for (i = 7; i >= 0; i--) 
  {
    MOSI_bit = (byte >> i) & 1;
    digitalWrite(MOSI_pin,MOSI_bit);  // write the bit
    digitalWrite(SCLK_pin,1);  // pull SCLK high
    MISO_bit = digitalRead(MISO_pin); // latch bit from MISO
    byteRead = (byteRead << 1) + MISO_bit;
    digitalWrite(SCLK_pin,0);  // pull SCLK low (shift MOSI and MISO registers)
  }

  return byteRead;

}

void readBlock()
{
  int i;
  unsigned char dummy_byte = 0xFF;
  unsigned char gotAByte;

  while (gotAByte = 0xFF) {};
  printf("got byte %02X\n",gotAByte);
  return;

  for (i = 0; i < 512; i++) 
  {
    // fill readBuffer  
  }  

  unsigned char CRCbyteHi = RWByte(dummy_byte);
  unsigned char CRCbyteLo = RWByte(dummy_byte);

  return;

}

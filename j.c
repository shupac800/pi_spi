#include <stdio.h>   // for printf
#include <string.h>  // for memcpy
#include <wiringPi.h>

// function declarations
void initializePi();
int getResponse();
void sendByteArr(unsigned char *byteArr, int length);
unsigned char RWBit(unsigned char bit);
int readBlock();

// globals
unsigned char MOSI_pin = 12;
unsigned char MISO_pin = 13;
unsigned char SCLK_pin = 14;
unsigned char CE0_pin  = 10;

void main()
{
  unsigned char byteArr[6] = { 0 };
  unsigned char cmd0[6]  = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
  unsigned char cmd1[6]  = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xFF };
  unsigned char cmd8[6]  = { 0x48, 0x00, 0x00, 0x01, 0xAA, 0x0F };
  unsigned char cmd16[6] = { 0x50, 0x00, 0x00, 0x02, 0x00, 0xFF };  // 0x00000200 = 512
  unsigned char cmd17[6] = { 0x51, 0x00, 0x00, 0x00, 0x00, 0xFF };
  unsigned char cmd41[6] = { 0x69, 0x00, 0x10, 0x00, 0x00, 0xE5 };
  unsigned char cmd55[6] = { 0x77, 0x00, 0x00, 0x00, 0x00, 0x65 };
  unsigned char cmd58[6] = { 0x7A, 0x00, 0x00, 0x00, 0x00, 0x75 };
  unsigned char cmd59[6] = { 0x7B, 0x00, 0x00, 0x00, 0x00, 0x91 };

  initializePi();

  // send CMD0
  memcpy(byteArr,cmd0,6);  // copy cmd0 bytes into byteArr
  sendByteArr(byteArr,6);
  printf("CMD0 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD55 -- receiving response 0x05, why??
  memcpy(byteArr,cmd55,6);
  sendByteArr(byteArr,6);
  printf("CMD55 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD41 -- receiving response 0x09 -- CRC error, idle state
  memcpy(byteArr,cmd41,6);
  sendByteArr(byteArr,6);
  printf("CMD41 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD8 -- usually generates response 0x05, "illegal command," which is fine
  memcpy(byteArr,cmd8,6);
  sendByteArr(byteArr,6);
  printf("CMD8 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD58
  memcpy(byteArr,cmd58,6);
  sendByteArr(byteArr,6);
  printf("CMD58 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD59 to turn off CRC
  memcpy(byteArr,cmd59,6);
  sendByteArr(byteArr,6);
  printf("CMD59 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete


  // send CMD1 -- receiving response 0x09 -- CRC error, idle state
  memcpy(byteArr,cmd1,6);
  sendByteArr(byteArr,6);
  printf("CMD1 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD17 -- receiving response 0x05, why??
  memcpy(byteArr,cmd17,6);
  sendByteArr(byteArr,6);
  printf("CMD17 generates response %02X\n",getResponse());
  readBlock();
  digitalWrite(CE0_pin,1);  // SPI transaction complete

  // idle state
  digitalWrite(MOSI_pin,1);
  digitalWrite(SCLK_pin,0);
  digitalWrite(CE0_pin,1);
  return;
}

void sendByteArr(unsigned char *byteArr, int length)
{
  int i, j;
  unsigned char MOSI_bit, MISO_bit, MISO_byte;

  digitalWrite(CE0_pin,0);  // open SPI transaction; ready to send command

  for (j = 0; j < length; j++) {  // j is index of byte being sent

    MISO_byte = 0xFF;  // initialize new byte

    for (i = 7; i >= 0 ; i--)
    {
      MOSI_bit = (byteArr[j] >> i) & 1;
      MISO_bit = RWBit(MOSI_bit);  // send MOSI bit, toggle clock, latch MISO bit
      MISO_byte = (MISO_byte << 1) + MISO_bit;
    }
  }

  digitalWrite(MOSI_pin,1); // MOSI is held high when not sending

  return;
}

void initializePi()
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
  for (i = 0; i < 74; i++)
  {
    digitalWrite(SCLK_pin,1);  // toggle SCLK for at least 74 cycles
    delayMicroseconds(1);
    digitalWrite(SCLK_pin,0);  // leave SCLK low
  }

  return;
}

int getResponse()
{
  int i, j;
  unsigned char MISO_byte;

  // wait for response
  for (i = 0; i < 16; i++)  // wait 16 clock cycles
  {
    digitalWrite(SCLK_pin,1);
    delayMicroseconds(1);
    digitalWrite(SCLK_pin,0);
    unsigned char MISO_bit = digitalRead(MISO_pin);
    if (!MISO_bit) {  // return message commences when MISO goes low
      for (j = 0; j < 8; j++)  // get 8 bits of response (first bit always 0)
      {
        MISO_byte = (MISO_byte << 1) + MISO_bit;
        digitalWrite(SCLK_pin,1);  // toggle SCLK
        delayMicroseconds(1);
        digitalWrite(SCLK_pin,0);
        MISO_bit = digitalRead(MISO_pin); // read next bit
      }
      return MISO_byte;  // MISO_byte is first (and maybe only) byte received in response to command
    }
  } 
  return -1; // no response received
}

unsigned char RWBit(unsigned char MOSI_bit)
{
  digitalWrite(MOSI_pin,MOSI_bit);  // write the bit
  digitalWrite(SCLK_pin,1);  // pull SCLK high
  delayMicroseconds(1);
  unsigned char MISO_bit = digitalRead(MISO_pin); // latch bit from MISO
  digitalWrite(SCLK_pin,0);  // pull SCLK low (shift MOSI and MISO registers)

  return MISO_bit;
}

int readBlock()
{
  int i, j;
  unsigned char MISO_bit, MISO_byte;
  // read bytes until you get 0xFE (data token) or 0x0? (error)
  // how do you know where bytes align?

  for (j = 0; j < 2560; j++)  // read 256 bytes
  {
    for (i = 7; i >= 0 ; i--)
    {
      MISO_bit = RWBit(1);  // send dummy bit, toggle clock, latch MISO bit
      MISO_byte = (MISO_byte << 1) + MISO_bit;
    }
    if (MISO_byte != 0xFF) 
    {
      printf("After %d byte reads, got non-FF byte %02X\n",j,MISO_byte);
      return MISO_byte;
    }
  }
  printf("After %d byte reads, did not find non-FF byte\n",j);
  return -1;
}
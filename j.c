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
  unsigned char byteArr[7] = { 0 };
  unsigned char cmd0[7]  = { 0xFF, 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
  unsigned char cmd1[7]  = { 0xFF, 0x41, 0x00, 0x00, 0x00, 0x00, 0xFF };
  unsigned char cmd8[7]  = { 0xFF, 0x48, 0x00, 0x00, 0x01, 0xAA, 0x0F };
  unsigned char cmd10[7] = { 0xFF, 0x4A, 0x00, 0x00, 0x02, 0x00, 0xFF };
  unsigned char cmd16[7] = { 0xFF, 0x50, 0x00, 0x00, 0x02, 0x00, 0xFF };  // 0x00000200 = 512
  unsigned char cmd17[7] = { 0xFF, 0x51, 0x00, 0x00, 0x00, 0x00, 0xFF };
  unsigned char cmd41[7] = { 0xFF, 0x69, 0x00, 0x10, 0x00, 0x00, 0xE5 };
  unsigned char cmd55[7] = { 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x65 };
  unsigned char cmd58[7] = { 0xFF, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x75 };
  unsigned char cmd59[7] = { 0xFF, 0x7B, 0x00, 0x00, 0x00, 0x00, 0x91 };

  initializePi();

  // send CMD0
  memcpy(byteArr,cmd0,7);  // copy cmd0 bytes into byteArr
  sendByteArr(byteArr,7);
  printf("CMD0 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete
  delayMicroseconds(1000000);

  // send CMD58
  memcpy(byteArr,cmd58,7);
  sendByteArr(byteArr,7);
  printf("CMD58 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete
  delayMicroseconds(1000000);

  unsigned char response = 0xFF;
  unsigned char throwaway = 0xFF;
  printf("Doing CMD58+CMD41 loop...\n");
  while ( 
//          ( response != 0x01 ) &&
          ( response != 0x00 ) &&
//          ( response != 0x05 ) &&
          ( response != 0x09 ) )
  {
    // send CMD55
    memcpy(byteArr,cmd55,7);
    sendByteArr(byteArr,7);
    //printf("CMD55 generates response %02X\n",getResponse());
    throwaway = getResponse();
    digitalWrite(CE0_pin,1);  // SPI transaction complete
    delayMicroseconds(10000);

    // send CMD41
    memcpy(byteArr,cmd41,7);
    sendByteArr(byteArr,7);
    response = getResponse();
    //printf("CMD41 generates response %02X\n",getResponse());
    digitalWrite(CE0_pin,1);  // SPI transaction complete
    delayMicroseconds(10000);
  }
  //printf("CMD58 generated response %02X\n",throwaway);
  printf("CMD41 generated response %02X\n",response);

  // // send CMD8
  // memcpy(byteArr,cmd8,7);
  // sendByteArr(byteArr,7);
  // printf("CMD8 generates response %02X\n",getResponse());
  // digitalWrite(CE0_pin,1);  // SPI transaction complete

  // // send CMD59 to turn off CRC
  // memcpy(byteArr,cmd59,7);
  // sendByteArr(byteArr,7);
  // printf("CMD59 generates response %02X\n",getResponse());
  // digitalWrite(CE0_pin,1);  // SPI transaction complete

  // // send CMD1
  // memcpy(byteArr,cmd1,7);
  // sendByteArr(byteArr,7);
  // printf("CMD1 generates response %02X\n",getResponse());
  // digitalWrite(CE0_pin,1);  // SPI transaction complete

  // // send CMD10
  // memcpy(byteArr,cmd10,7);
  // sendByteArr(byteArr,7);
  // printf("CMD10 generates response %02X\n",getResponse());
  // digitalWrite(CE0_pin,1);  // SPI transaction complete

  // send CMD17
  memcpy(byteArr,cmd17,7);
  sendByteArr(byteArr,7);
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
    delayMicroseconds(1);
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
    delayMicroseconds(1);
    unsigned char MISO_bit = digitalRead(MISO_pin);
    if (!MISO_bit) {  // return message commences when MISO goes low
      for (j = 0; j < 8; j++)  // get 8 bits of response (first bit always 0)
      {
        MISO_byte = (MISO_byte << 1) + MISO_bit;
        digitalWrite(SCLK_pin,1);  // toggle SCLK
        delayMicroseconds(1);
        digitalWrite(SCLK_pin,0);
        delayMicroseconds(1);
        MISO_bit = digitalRead(MISO_pin); // read next bit
      }
      return MISO_byte;  // MISO_byte is first (and maybe only) byte received in response to command
    }
  } 
  return -1; // no response received
}

unsigned char RWBit(unsigned char MOSI_bit)
{
  digitalWrite(MOSI_pin,MOSI_bit);  // write the outgoing bit to MOSI
  digitalWrite(SCLK_pin,1);  // pull SCLK high
  delayMicroseconds(1);
  unsigned char MISO_bit = digitalRead(MISO_pin); // latch incoming bit from MISO
  digitalWrite(SCLK_pin,0);  // pull SCLK low (shift MOSI and MISO registers)
  delayMicroseconds(1);

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
    MISO_byte = 0xFF;
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
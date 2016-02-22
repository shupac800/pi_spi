#include <stdio.h>   // for printf
#include <string.h>  // for memcpy
#include <wiringPi.h>

// function declarations
void initializePi();
int getResponse();
void sendByteArr(unsigned char *byteArr, int length);
unsigned char RWBit(unsigned char bit);
unsigned char readData();

// globals
unsigned char MOSI_pin = 12;
unsigned char MISO_pin = 13;
unsigned char SCLK_pin = 14;
unsigned char CE0_pin  = 10;

void main()
{
  unsigned char byteArr[7] = { 0 };
  unsigned char cmd0[7]  = { 0xFF, 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
  unsigned char cmd16[7] = { 0xFF, 0x50, 0x00, 0x00, 0x02, 0x00, 0xFF };  // 0x00000200 = 512
  unsigned char cmd17[7] = { 0xFF, 0x51, 0x11, 0x8E, 0x48, 0x00, 0xFF }; // note: must start reading on 512-byte boundary
  unsigned char cmd24[7] = { 0xFF, 0x58, 0x00, 0x01, 0x1A, 0x00, 0xFF };
  unsigned char cmd41[7] = { 0xFF, 0x69, 0x00, 0x10, 0x00, 0x00, 0xE5 };
  unsigned char cmd55[7] = { 0xFF, 0x77, 0x00, 0x00, 0x00, 0x00, 0x65 };
  unsigned char cmd58[7] = { 0xFF, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x75 };

  unsigned char buffer[515] = { 0 };

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
          ( response != 0x00 ) &&
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

  // send CMD16, set block length
  memcpy(byteArr,cmd16,7);
  sendByteArr(byteArr,7);
  printf("CMD16 generates response %02X\n",getResponse());
  digitalWrite(CE0_pin,1);  // SPI transaction complete
  delayMicroseconds(10000);

  unsigned char doLoop = 1;
  unsigned long address = 0x10000000;
  int i;
  while (doLoop)
  {
    // send CMD17 with current address
    cmd17[2] = address >> 24 & 0xFF;
    cmd17[3] = address >> 16 & 0xFF;
    cmd17[4] = address >> 8 & 0xFF;
    cmd17[5] = address & 0xFF;
    memcpy(byteArr,cmd17,7);
    sendByteArr(byteArr,7);
    printf("CMD17 generates response %02X\n",getResponse());
    printf("reading 512 bytes at address %08X\n",address);
    *buffer = readData();
    digitalWrite(CE0_pin,1);  // SPI transaction complete
    // send 8 extra clocks
    RWBit(1);
    RWBit(1);
    RWBit(1);
    RWBit(1);
    RWBit(1);
    RWBit(1);
    RWBit(1);
    RWBit(1);

    for (i = 0; i < 513; i++)
    {
      printf("%02X ",buffer[i]);
    }
    printf("CRC bytes: %02X %02X\n",buffer[513],buffer[514]);
    if ((buffer[513] != 0) || (buffer[514] != 0))  // checksum byte is non-zero?
    {
      doLoop = 0;  // stop looping if non-zero checksum byte
    }
    else
    {
      address += 512;
    }
  }

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
  digitalWrite(SCLK_pin,0);  // pull SCLK low 
  delayMicroseconds(1);

  return MISO_bit;
}

unsigned char readData()
{
  int i, j;
  unsigned char MISO_bit, MISO_byte;
  unsigned char buffer[515];

  // read bytes until you get data token 0xFE (data token) or 0x0? (error)
  for (j = 0; j < 256; j++)  // read 256 bytes
  {
    MISO_byte = 0xFF;
    for (i = 7; i >= 0 ; i--)
    {
      MISO_bit = RWBit(1);  // send dummy bit, toggle clock, latch MISO bit
      MISO_byte = (MISO_byte << 1) + MISO_bit;
    }
    if (MISO_byte != 0xFF)
    {
      printf("GOT TOKEN %02X after %d byte reads\n",MISO_byte,j);
      j = 256;  // exit loop
    }
  }
  if (MISO_byte == 0xFF) {
    printf("After %d byte reads, did not find non-FF byte\n",j);
    return -1;
  }
  buffer[0] = MISO_byte;  // put data token in byte 0 of array

  for (j = 1; j < 515; j++) // get 512 data bytes and 2 checksum bytes
  {
    MISO_byte = 0xFF;
    for (i = 7; i >= 0; i--)
    {
      MISO_bit = RWBit(1);  // send dummy bit, toggle clock, latch MISO bit
      MISO_byte = (MISO_byte << 1) + MISO_bit;
    }
    buffer[j] = MISO_byte;
  }

  return *buffer;
  
}


#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// function declarations
int initializePiGPIO();
int sendFrame(unsigned char *data, int length);

int main()
{
  int byte = { 0xAA };
  sendByte(byte);
  return 0;
}

int sendByte(int byte) {
  int i, bit;

  printf("sendByte received data %02x\n\n",byte);

  for (i = 7; i >= 0 ; i--) {
   printf("wha\n");
    bit = (byte >> i) & 1;
    printf("  bit %d is %d\n",7,bit);
  }

  return 0;
}

int initializePiGPIO()
{
  wiringPiSetup();  // initialize GPIO
  int fd = wiringPiSPISetupMode(0,500000,0); // initialize SPI channel 0
  if (fd <= -1)
  {
    printf("SPI setup failed\n");
    return 1; 
  }
  printf("wiringPiSPISetup returned status = %d\n",fd);
  return 0;
}


#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// function declarations
int initializePiGPIO();
int sendByteArr(int *byteArr, int length);

int main()
{
  int byteArr[4] = { 0xAA, 0x01, 0x02, 0x03 };
  int length = sizeof *byteArr;
  sendByteArr(byteArr,length);
  return 0;
}

int sendByteArr(int *byteArr, int length) {
  int i, j, bit;

  for (j = 0; j < length; j++) {

    printf("sendByte received data %02x\n\n",byteArr[j]);

    for (i = 7; i >= 0 ; i--) {
      bit = (byteArr[j] >> i) & 1;
      printf("  bit %d is %d\n",7,bit);
    }

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

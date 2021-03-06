//
// simple program to test binary file-reading
//

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// function declarations
int initializePiGPIO();
int sendFrame(unsigned char *data, int length);

// global declarations
int MOSI_pin = 12;
int MISO_pin = 13;
int SCLK_pin = 14;
int CE0_pin = 10;

int main()
{
  initializePiGPIO();

  return 0;
}

int sendFrame(unsigned char *data, int length) {
  wiringPiSPIDataRW(0, data, length);
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

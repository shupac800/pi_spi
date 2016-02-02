//
// simple program to test binary file-reading
//

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// function declarations
int initializePiGPIO();
int sendFrame(unsigned char *data, int length);

int main()
{
  initializePiGPIO();

  char data[4];
  *(data + 0) = 66;
  *(data + 1) = 66;
  *(data + 2) = 66;
  *(data + 3) = 66;
  sendFrame(data,4);
  return 0;
}

int sendFrame(unsigned char *data, int length) {
  printf("sendFrame received %s, %d\n",data,length);
  wiringPiSPIDataRW(0, data, length);
  printf("received data: %s\n",data);
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

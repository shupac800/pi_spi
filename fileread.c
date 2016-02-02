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
  FILE *file_ptr;
  unsigned char *buffer;
  unsigned long lSize;
  int i;

  file_ptr = fopen("136025.109","rb");
  if (!file_ptr)
  {
    printf("Unable to open input file");
    return 1;
  }

  //Get file length
  fseek(file_ptr, 0, SEEK_END);
  lSize = ftell(file_ptr); // lSize = 16384 for 16k file
  fseek(file_ptr, 0, SEEK_SET);

  buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);

  //Read file contents into buffer
  fread(buffer, 1, lSize, file_ptr);
  fclose(file_ptr);

  // print 128 characters
  for (i = 16; i < 32; i++)
  {
    // if character not printable, print "."
    // putc( isprint(buffer[i]) ? buffer[i] : '.' , stdout );
    printf("%02x ",buffer[i]); // print hex value
  }
  printf("\n");
  initializePiGPIO();


  buffer = "Greetings Professor Falken";
  lSize = 26;
  printf("%s\n",buffer);
  int q = sendFrame(1,1);
  printf("q: %s\n",q);
  return 0;
}

int initializePiGPIO()
{
  wiringPiSetup();  // initialize GPIO
  int fd = wiringPiSPISetup(0,1000000); // initialize SPI channel 0 at 1 MHz
  if (fd <= -1)
  {
    printf("SPI setup failed\n");
    return 1; 
  }
  // gpio readall
  // remember, GPIO access requires sudo or root
  // remember to complie using -lwiringPi
  // SPI pinout of Pi A+:
  //              3V3 = physical 15
  //    wPi 12 = MOSI = physical 19 (Master Out, Slave In)
  //    wPi 13 = MISO = physical 21
  //    wPi 14 = SCLK = physical 23
  //    wPi 10 =  CE0 = physical 24 (active HIGH)
  //              GND = physical 25
  //    wPi 11 =  CE1 = physical 26
  
  // it’s possible to use the SPI bus for things that weren’t designed for SPI – 
  // one example is shift registers. Just use the clock and MOSI outputs, and write
  // a byte at a time to the SPI device.

  return 0;
}

// int wiringPiSPISetup (int channel, int speed);

// This is the way to initialise a channel (The Pi has 2 channels; 0 and 1). The speed 
// parameter is an integer in the range 500,000 through 32,000,000 and represents the SPI
// clock speed in Hz.

// The returned value is the Linux file-descriptor for the device, or -1 on error. If an 
// error has happened, you may use the standard errno global variable to see why.

int sendFrame(unsigned char *data, int length) {
  return wiringPiSPIDataRW(0, data, length);
}

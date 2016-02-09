
#include <stdio.h>
#include <wiringPi.h>

// function declarations
int initializePi();
int sendByteArr(int *byteArr, int length);

// globals
  int MOSI_pin = 19;
  int SCLK_pin = 23;
  int CE0_pin = 24;

int main()
{
  wiringPiSetup();
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(14,OUTPUT);
  pinMode(10,OUTPUT);
  while(1) {
    digitalWrite(14,HIGH);
    delayMicroseconds(1);
    digitalWrite(14,LOW);
    delayMicroseconds(1);
  }

}

int sendByteArr(int *byteArr, int length) {
  int i, j, bit;

  for (j = 0; j < length; j++) {

    printf("sendByte received data %02x\n\n",byteArr[j]);

    for (i = 7; i >= 0 ; i--) {
      bit = (byteArr[j] >> i) & 1;
      printf("  bit %d is %d\n",7,bit);
      sendBit(bit);
    }

  }

  return 0;
}

int initializePi()
{
  return 0;
}

int sendBit(int bit) {
  
  digitalWrite(CE0_pin,LOW);  // pull enable low
  digitalWrite(SCLK_pin,LOW);  // pull SCLK low
  delayMicroseconds(1);
  if (bit)
  {
    digitalWrite(MOSI_pin,HIGH);  // write the bit
  }
  else
  {
    digitalWrite(MOSI_pin,LOW);  // write the bit
  }
  delayMicroseconds(1);
  digitalWrite(SCLK_pin,HIGH);  // pull SCLK high
  digitalWrite(CE0_pin,HIGH);  // pull enable high

  return 0;

}

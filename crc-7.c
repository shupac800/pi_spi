#include <stdio.h>

unsigned char CRCPoly = 0x89;  // the value of our CRC-7 polynomial, 10001001
unsigned char CRCTable[256];
 
void GenerateCRCTable()
{
    int i, j;
 
    // generate a table value for all 256 possible byte values
    for (i = 0; i < 256; i++)
    {
        CRCTable[i] = (i & 0x80) ? i ^ CRCPoly : i;
        for (j = 1; j < 8; j++)
        {
            CRCTable[i] <<= 1;
            if (CRCTable[i] & 0x80)
                CRCTable[i] ^= CRCPoly;
        }
    }
}
 
 
// adds a message byte to the current CRC-7 to get a the new CRC-7
unsigned char CRCAdd(unsigned char CRC, unsigned char message_byte)
{
    return CRCTable[(CRC << 1) ^ message_byte];
}
 
 
// returns the CRC-7 for a message of "length" bytes
unsigned char getCRC(unsigned char message[], int length)
{
    int i;
    unsigned char CRC = 0;
 
    for (i = 0; i < length; i++)
        CRC = CRCAdd(CRC, message[i]);
 
    return CRC;
}

void main() 
{
  unsigned char cmd = 1;
  unsigned char buffer[5] = { 0x7B, 0x00, 0x00, 0x00, 0x00 };

  GenerateCRCTable(); 
  unsigned char crc = getCRC(buffer,5);
  unsigned char lastByte = (crc << 1) + 1;

  printf("Command %d generated last byte of %02X\n",cmd,lastByte);

  return;
}

   /* CRC 7 to produce SD memory card packet format

      Code works as tested for one packet example given by
      student questioning the algorithm.
      Code derived from documentation of Maxim website,
      as detailed below.

      Code may not be pretty or optimized, but it does
      do the job. Code was compiled using Borland C compiler
      for DOS, with DOS cmnd line executable (included in ZIP file)
      runnable under Windows XP via command line.

      Usage is
      CRC_7 Val1 Val2 Val3 Val4 Val5
        where Val1..5 are decimal values for the bytes of command to SD.
      For example, CRC_7 64 0 0 0 0
        generates the data for the student's example to send GO_IDLE_STATE
         command (lower 6 bits of 0).

         Any and all use of this code is granted to any and all users.
       To contact the author of this code, send an email to:

      bitwacker55@yahoo.com

         I'll try to be of assistance. I also program in LabView, Visual
         Basic, Python, Forth, etc.

      Information from ZIP file to authenticate EXE file.

      2008-07-24  10:35       7900 1CD57D4B CRC_7.EXE
      7e2d8ec62f7f5500e830b954c0a7f651  crc_7.exe   MD5 digest.

   */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/*
CRC7 algorithm info :


http://www.pldesignline.com/showArticle.jhtml?articleID=209102269&cid=NL_pldl


Student in Canada has question in above website.

Frame below (cmd 0 as 0x40 Go Idle State)

01000000 00000000 00000000 00000000 00000000 10010101


Text cut from web site:
http://www.maxim-ic.com/appnotes.cfm/an_pk/3969

Cyclic Redundancy Check
The CRC algorithm is commonly used to detect errors induced by an unreliable 
communication channel. The selection of a particular CRC is governed by 
the size of data to be protected. In the case of SD media, CRC-7 and CRC-16 
are specified.

The CRC algorithm divides the protected data by a selected divisor and 
produces a remainder. This division is done without carry logic due to 
the polynomial math used in the algorithm. As no carries are needed, division 
can be accomplished with the logical XOR operation. The selected divisor 
is commonly referred to as the CRC's polynomial. The resulting remainder 
is then transmitted with the data, and can be used by the receiver to 
check that the data was not corrupted during transmission.

In the case of CRC-7, the remainder can be calculated using a 7-bit shift 
register in software. This shift register is initialized to all zeros 
at the start of the calculation. As each bit (MSB first) of the protected 
data is shifted into the LSB of the shift register, the MSB of the shift 
register is shifted out and examined. If the bit just shifted out is one, 
the contents of the shift register are modified by XORing with the CRC-7 
polynomial value 0x09. If the bit shifted out of the shift register is 
zero, no XOR is performed. Once the last bit of protected data is shifted 
in and the conditional XOR completed, seven more zeros must be shifted 
through in a similar manner. This process is referred to as augmentation, 
and completes the polynomial division. At this point, the CRC-7 value 
can be read directly from the shift register.


SD Command Format
Commands are issued to the card in a 6-byte format (Identified as Bytes 1..6).
The first byte of a command can be constructed by ORing the 6-bit command code with
hex 0x40. The next four bytes provide a single 32-bit argument, if
required by the command; the final byte contains the CRC-7 checksum over
bytes 1 through 5. Table 1 lists important SD commands.

Byte1.2.3.4.5.6 with 6 being CRC with 7 bits in MS order rank, with LSB of 1
added at the end. After Byte 5 is CRCed an augment of 7 zero bits is mixed
in too.


Table 1. Selected SD Memory Card Commands
Command    Mnemonic             Argument Reply  Description
0 (0x00)   GO_IDLE_STATE        none     R1     Resets the SD card.
9 (0x09)   SEND_CSD             none     R1     Sends card-specific data.
10 (0x0a)  SEND_CID             none     R1     Sends card identification.
17 (0x11)  READ_SINGLE_BLOCK    address  R1     Reads a block at byte address.
24 (0x18)  WRITE_BLOCK          address  R1     Writes a block at byte address.
55 (0x37)  APP_CMD              none     R1     Prefix for application command.
59 (0x3b)  CRC_ON_OFF           Only Bit 0 R1   Argument sets CRC on (1) or off (0).
41 (0x29)  SEND_OP_COND         none     R1     Starts card initialization.


*/

/****************/
static unsigned char Encode( unsigned char Seed, unsigned char Input , unsigned char Depth)
                  /* --
                     Produce a 7 bit CRC value Msb first.
                    Seed is last round or initial round shift register
                      value (lower 7 bits significant).
                    Input is an 8 bit value from byte stream being CRCd.
                    CRC register is seeded initially with the value 0 prior
                     to mixing in the variable packet data.
                    Depth is usually 8, but the last time is 7 to shift in
                      the augment string of 7 zeros.
*****************/
{
/*begin-Encode. -
      local defs: */
   register unsigned char regval;      // shift register byte.
   register unsigned char count;
   register unsigned char cc;          // data to manipulate.
 #define POLYNOM (0x9)        // polynomical value to XOR when 1 pops out.

/*BODY*/

   regval = Seed;    // get prior round's register value.
   cc = Input;       // get input byte to generate CRC, MSB first.

   /* msb first of byte for Depth elements */
   for ( count = Depth     // set count to 8 or 7.
          ; count--        // for count # of bits.
           ;  cc <<= 1     // shift input value towards MSB to get next bit.
       )
      {
      // Shift seven bit register left and put in MSB value of input to LSB.
      regval = (regval << 1) + ( (cc & 0x80) ? 1 : 0 );
      // Test D7 of shift register as MSB of byte, test if 1 and therefore XOR.
      if (regval & 0x80)
         regval ^= POLYNOM;
      } // end byte loop.
   return (regval & 0x7f);    // return lower 7 bits of CRC as value to use.
}
/*...
end-Encode.
.....*/




int main(int argc, char * argv[] )
{
   char * cp;     // pointer into command line argument list.
   int walker;    // index into argv parameter list.
   int value, nuval;    // temporary values from cmd line and CRC generator.
   unsigned char CrcAccum; // local storage of each round of CRC.

   // 7 bit shift register CRC computation across 5 byte SD message field
   //   with 6th byte of message being the CRC field generated herein.
   if ( argc != (1+5) )
      {
      fprintf(stderr, "\nCRC_7 command line syntax error.\n");
      fprintf(stderr, "Requires 5 argument(s) to encode w/crc value\n");
      fprintf(stderr, "Usage is :\nCRC_7 0 0 0 0 0\n");
      fprintf(stderr, " or similar with decimal value for each byte.\n");
      fprintf(stderr, "  Use CRC_7 64 0 0 0 0\n   to generate example run.\n\n");
      return 1;
 // EARLY EXIT, CMND LINE SYNTAX ERROR
      }

   // start w/seed of 0 per algorithm.
   CrcAccum = 0;

   // loop through 5 command line decimal values. Display in HEX to
   //  stdout, can be captured via >> on cmdline.
   for ( walker = 1 ; walker < argc ; walker++ )
      {
      cp = argv[walker];
      value = atoi(cp);
      nuval = Encode(CrcAccum, value, 8 );
      printf(" CRC remainder of 0x%02X for 0x%02X input byte.\n"
               , nuval
               , value
            );
      CrcAccum = nuval;    // reload crc accum.
      }

   // mix in last 7 bits of 0s to augment, and then shift final CRC
   //  remainder left and OR in fixed 1 LSB.
   nuval = Encode(CrcAccum, 0, 7 );
   value = (nuval << 1) + 1;
   printf("CRC remainder after augment is 0x%02X with FINAL CRC byte of 0x%02X.\n"
               , nuval
               , value
            );
   return 0;
}


/* end of file */


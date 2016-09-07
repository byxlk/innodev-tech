#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int8_t MuLaw_Encode(int16_t number)
{
	const uint16_t MULAW_MAX = 0x1FFF;
	const uint16_t MULAW_BIAS = 33;
	uint16_t mask = 0x1000;
	uint8_t sign = 0;
	uint8_t position = 12;
	uint8_t lsb = 0;
	if (number < 0)
	{
		number = -number;
		sign = 0x80;
	}
	   number += MULAW_BIAS;
	   if (number > MULAW_MAX)
	   {
		   number = MULAW_MAX;
	   }
	      for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--)
			  ;
		  lsb = (number >> (position - 4)) & 0x0f;
		  return (~(sign | ((position - 5) << 4) | lsb));
}

int16_t MuLaw_Decode(int8_t number)
{
	const uint16_t MULAW_BIAS = 33;
	uint8_t sign = 0, position = 0;
	int16_t decoded = 0;
	number = ~number;
	if (number & 0x80)
	{
		number &= ~(1 << 7);
		sign = -1;
	}
	   position = ((number & 0xF0) >> 4) + 5;
	   decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
	   | (1 << (position - 5))) - MULAW_BIAS;
	   return (sign == 0) ? (decoded) : (-(decoded));
}

#ifndef MAIN
int main(int argc, char**argv)
{
	if(argc < 2) {
		printf("Usage: %s in.ulaw out.u8\n", argv[0]);
		exit(-1);
	}
	int8_t in[256];
	uint8_t out8[256];
	int16_t out[256];
	int i, s;
	
	FILE *fp_in = fopen(argv[1], "r");
	FILE *fp_out = fopen(argv[2], "w");
	
	while(!feof(fp_in)) {
		s = fread(in, sizeof(int8_t), 256, fp_in);
		printf("fread: s=%d\n", s);
		for(i=0;i<s;i++) {
			out[i] = MuLaw_Decode(in[i]); // s16
			out8[i] = (MuLaw_Decode(in[i]) >> 6) + 128; // u8, 正常應該 >> 8
			//out[i] = MuLaw_Decode(in[i])+5000; // unsigned
			printf("%d: %d\n", i, out8[i]);
		}
		//fwrite(out, sizeof(int16_t), s, fp_out);
		fwrite(out8, sizeof(int8_t), s, fp_out);
	}
}
#endif
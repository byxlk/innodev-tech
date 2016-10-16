#ifndef WAVE_H
#define WAVE_H

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>


typedef struct _RIFF_HEADER { 
  char           szRiffID[5];  // 'R','I','F','F'   
  unsigned int   dwRiffSize;   // wav file length 
  char           szRiffFormat[5]; // 'W','A','V','E'  
} RIFF_HEADER_T;

typedef struct _WAV_FORMAT {
  char           szFmtID[5];   // 'f', 'm', 't'
  unsigned long  dwFmtSize;
  short          wFormatTag;
  unsigned short wChannels;
  unsigned long  dwSamplesPerSec;
  unsigned long  dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;
/* note: there may be additional fields here, depending upon wformattag. */

} WAV_FORMAT_T;

typedef struct _WAV_FACT {
  char           szFactID[5];
  unsigned int   dwFactSize;
} WAV_FACT_T;

typedef struct _WAV_DATA{
  char           szDataID[5];
  unsigned int   dwDataSize;
} WAV_DATA_T;


typedef struct _WAV_T {
  FILE           *fp;
  RIFF_HEADER_T  riff;
  WAV_FORMAT_T   format;
  WAV_FACT_T     fact;
  WAV_DATA_T     data;
  int            file_size;
  int            data_offset;
  int            data_size;
} WAV_T;




WAV_T *wav_open(char *file_name);
void wav_close(WAV_T **wav);
void wav_rewind(WAV_T *wav);
int wav_over(WAV_T *wav);
int wav_read_data(WAV_T *wav, char *buffer, int buffer_size);
void wav_dump(WAV_T *wav);


#if 0
int load_wave(char *file, FORMAT *fmt, void *data, int *data_size);
int save_wave(char *file, FORMAT *fmt, void *data, int data_size);
#endif

#endif

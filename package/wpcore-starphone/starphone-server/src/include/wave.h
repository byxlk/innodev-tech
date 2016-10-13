#ifndef WAVE_H
#define WAVE_H

typedef struct {
  short          wFormatTag;
  unsigned short wChannels;
  unsigned long  dwSamplesPerSec;
  unsigned long  dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;
/* note: there may be additional fields here, depending upon wformattag. */

} FORMAT;

int load_wave(char *file, FORMAT *fmt, void **data, int *data_size);
int save_wave(char *file, FORMAT *fmt, void *data, int data_size);

#endif

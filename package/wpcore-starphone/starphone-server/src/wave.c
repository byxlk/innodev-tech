#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "include/xw_export.h"

/* load_wave()
     load a wave file into memory.

     returns: 0 - successfully
              1 - file open error
              2 - file type not supported
              3 - memory full
*/
int load_wave(char *file, FORMAT *fmt, void **data, int *data_size)
{
  FILE *fp;
  char id[5];
  char temp[20];
  long size;
  int RIFF = 0, DATA = 0, FMT = 0;

  char *s;
  int len, b;

  if ((fp = fopen(file, "r")) == NULL)
	return 1;

  /* File begins with "RIFFxxxxWAVEfmt "... */
  fread(temp, 16, 1, fp);
  if (strncmp(temp, "RIFF", 4) != 0
	|| strncmp(temp + 8, "WAVE", 4) != 0
	|| strncmp(temp + 12, "fmt ", 4) != 0)
    return 2;
  fseek(fp, 0, SEEK_SET);
  
  // At the begining of a chunk
  while(!feof(fp))
  {
    // ID and Size
    fread(id, 4, 1, fp);
    id[4] = '\0';
    fread(&size, 4, 1, fp);

/*    printf ("ID=[%4s], size=%4ld\n", id, size); */
    if(strncmp(id, "RIFF", 4) ==0)
    {
//    fseek(fp, ftell(fp) + strlen(id) + sizeof(size) + strlen("WAVE"), SEEK_SET); 
      RIFF = 1;
      fseek(fp, strlen("WAVE"), SEEK_CUR); 
    }
    else if (strncmp(id, "fmt ", 4) == 0)
    {
      FMT = 1;
      fread(fmt, 1, sizeof(FORMAT), fp);
/*      printf ("fmt->wFormatTag:%d\n", fmt->wFormatTag); */
      if (fmt->wFormatTag != 1)
        return 2;

      fseek(fp, size - sizeof(FORMAT), SEEK_CUR); 
    }
    else if (strncmp(id, "data", 4) == 0)
    {
      DATA = 1;
      *data_size = size;
      *data = (void *)malloc(size);
      if(*data == NULL)
        return 3;

      b = fread(*data, 1, size, fp);
/*     printf ("b=%d\n", b); */
    }
    else if(strncmp(id, "LIST", 4) ==0)
      ;
    else
      fseek(fp, size, SEEK_CUR); 

    if (RIFF && DATA && FMT)
	break;
  }

  fclose(fp);
  return 0;
}

/* save_wave()
     save a wave struct into disk file.

     returns: 0 - successfully
              1 - file write error
*/
int save_wave(char *file, FORMAT *fmt, void *data, int data_size)
{
  FILE *fp;
  int d;

  void *s;
  long b, len;

  if ((fp = fopen(file, "w")) == NULL)
	return 1;

/*  printf ("save_wave: data_size=%d\n", data_size); */

  fputs ("RIFF", fp);
  d = 36 + data_size;
  fwrite (&d, 1, sizeof(long), fp);
  fputs ("WAVEfmt ", fp);
  d = 16;
  fwrite(&d, sizeof(long), 1, fp);
  fwrite(fmt, sizeof(FORMAT), 1, fp);
  fputs ("data", fp);
  fwrite(&data_size, sizeof(long), 1, fp);

/*printf ("watered saved.\n\n"); */
//fp=fopen("aaaa.dat", "w");
  s = data; len = 0;
  while (len != data_size)
  {
/* printf ("write:"); */
    b = fwrite (s, 1, data_size, fp);
/* printf ("b=%d\n", b); */
    s += b; len += b;
    if (b <= 0) 
      break;
  }

  fclose(fp);
  return 0;
}

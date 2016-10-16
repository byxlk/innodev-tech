#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "include/xw_export.h"



WAV_T *wav_open(char *file_name)
{
    char buffer[256];
    int  read_len = 0;
    int  offset = 0;
    WAV_T *wav = NULL;


    if(NULL == file_name){
        printf("file_name is NULL\n");
        return NULL;
    }


    wav = (WAV_T *)malloc(sizeof(WAV_T));
    if(NULL == wav){
        printf("malloc wav failedly\n");
        return NULL;
    }
    bzero(wav, sizeof(WAV_T));

    wav->fp = fopen(file_name, "r");
    if(NULL == wav->fp){
        printf("fopen %s failedly\n", file_name);
        free(wav);
        return NULL;
    }


    //handle RIFF WAVE chunk
    read_len = fread(buffer, 1, 12, wav->fp);
        if(read_len < 12){
        printf("error wav file\n");
        wav_close(&wav);
        return NULL;
    }
    if(strncasecmp("RIFF", buffer, 4)){
        printf("error wav file\n");
        wav_close(&wav);
        return NULL;
    }
    memcpy(wav->riff.szRiffID, buffer, 4);
    wav->riff.dwRiffSize = *(int *)(buffer + 4);
    if(strncasecmp("WAVE", buffer + 8, 4)){
        printf("error wav file\n");
        wav_close(&wav);
        return NULL;
    }
    memcpy(wav->riff.szRiffFormat, buffer + 8, 4);
    wav->file_size = wav->riff.dwRiffSize + 8;


    offset += 12;


    while(1){
        char id_buffer[5] = {0};
        int  tmp_size = 0;


        read_len = fread(buffer, 1, 8, wav->fp);
        if(read_len < 8){
            printf("error wav file\n");
            wav_close(&wav);
            return NULL;
        }
        memcpy(id_buffer, buffer, 4);
        tmp_size = *(int *)(buffer + 4);


        if(0 == strncasecmp("FMT", id_buffer, 3)){
            memcpy(wav->format.szFmtID, id_buffer, 3);
            wav->format.dwFmtSize = tmp_size;
            read_len = fread(buffer, 1, tmp_size, wav->fp);
            if(read_len < tmp_size){
                printf("error wav file\n");
                wav_close(&wav);
                return NULL;
            }
            wav->format.wFormatTag       = *(short *)buffer;
            wav->format.wChannels        = *(short *)(buffer + 2);
            wav->format.dwSamplesPerSec  = *(int *)(buffer + 4);
            wav->format.dwAvgBytesPerSec = *(int *)(buffer + 8);
            wav->format.wBlockAlign      = *(short *)(buffer + 12);
            wav->format.wBitsPerSample   = *(short *)(buffer + 14);
        }
        else if(0 == strncasecmp("DATA", id_buffer, 4)){
            memcpy(wav->data.szDataID, id_buffer, 4);
            wav->data.dwDataSize = tmp_size;
            offset += 8;
            wav->data_offset = offset;
            wav->data_size = wav->data.dwDataSize;
            break;
        }
        else{
            printf("unhandled chunk: %s, size: %d\n", id_buffer, tmp_size);
            fseek(wav->fp, tmp_size, SEEK_CUR);
        }
        offset += 8 + tmp_size;
    }


    return wav;
}


void wav_close(WAV_T **wav)
{
    WAV_T *tmp_wav;

    if(NULL == wav){
        return ;
    }


    tmp_wav = *wav;
    if(NULL == tmp_wav){
        return ;
    }


    if(NULL != tmp_wav->fp){
        fclose(tmp_wav->fp);
    }
    free(tmp_wav);

    *wav = NULL;
}


void wav_rewind(WAV_T *wav)
{
    if(fseek(wav->fp, wav->data_offset, SEEK_SET) < 0){
        printf("wav rewind failedly\n");
    }
}


int wav_over(WAV_T *wav)
{
    return feof(wav->fp);
}


int wav_read_data(WAV_T *wav, char *buffer, int buffer_size)
{
    return fread(buffer, 1, buffer_size, wav->fp);
}


void wav_dump(WAV_T *wav)
{
    printf("file length: %d\n", wav->file_size);

    printf("\nRIFF WAVE Chunk\n");
    printf("id: %s\n", wav->riff.szRiffID);
    printf("size: %d\n", wav->riff.dwRiffSize);
    printf("type: %s\n", wav->riff.szRiffFormat);


    printf("\nFORMAT Chunk\n");
    printf("id: %s\n", wav->format.szFmtID);
    printf("size: %d\n", wav->format.dwFmtSize);
    
    if(wav->format.wFormatTag == 0){
        printf("compression: Unknown\n");
    }
    else if(wav->format.wFormatTag == 1){
        printf("compression: PCM/uncompressed\n");
    }
    else if(wav->format.wFormatTag == 2){
        printf("compression: Microsoft ADPCM\n");
    }
    else if(wav->format.wFormatTag == 6){
        printf("compression: ITU G.711 a-law\n");
    }
    else if(wav->format.wFormatTag == 7){
        printf("compression: ITU G.711 u-law\n");
    }
    else if(wav->format.wFormatTag == 17){
        printf("compression: IMA ADPCM\n");
    }
    else if(wav->format.wFormatTag == 20){
        printf("compression: ITU G.723 ADPCM (Yamaha)\n");
    }
    else if(wav->format.wFormatTag == 49){
        printf("compression: GSM 6.10\n");
    }
    else if(wav->format.wFormatTag == 64){
        printf("compression: ITU G.721 ADPCM\n");
    }
    else if(wav->format.wFormatTag == 80){
        printf("compression: MPEG\n");
    }
    else{
        printf("compression: Unknown\n");
    }

    printf("channels: %d\n", wav->format.wChannels);
    printf("samples: %d\n", wav->format.dwSamplesPerSec);
    printf("avg_bytes_per_sec: %d\n", wav->format.dwAvgBytesPerSec);
    printf("block_align: %d\n", wav->format.wBlockAlign);
    printf("bits_per_sample: %d\n", wav->format.wBitsPerSample);


    printf("\nDATA Chunk\n");
    printf("id: %s\n", wav->data.szDataID);
    printf("size: %d\n", wav->data.dwDataSize);
    printf("data offset: %d\n", wav->data_offset);
}

#if 0
/* load_wave()
     load a wave file into memory.

     returns: 0 - successfully
              1 - file open error
              2 - file type not supported
              3 - memory full
*/
int load_wave(char *file, FORMAT *fmt, void *data, int *data_size)
{
  FILE *fp;
  char id[5];
  char temp[20];
  long size;
  int RIFF = 0, DATA = 0, FMT = 0;

  void *s;
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

    printf ("ID=[%4s], size=%4ld\n", id, size); 
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
      printf ("fmt->wFormatTag:%d\n", fmt->wFormatTag); 
      if (fmt->wFormatTag != 1)
        return 2;

      fseek(fp, size - sizeof(FORMAT), SEEK_CUR); 
    }
    else if (strncmp(id, "data", 4) == 0)
    {
      DATA = 1;
      *data_size = size;
      data = (void *)malloc(size);
      if(data == NULL)
        return 3;

      b = fread(data, 1, size, fp);
      printf ("b=%d\n", b);
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

#endif

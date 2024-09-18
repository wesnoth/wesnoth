/*
 * This program is for setting TTF files to Installable Embedding mode.
 *
 * Note that using this to embed fonts which you are not licensed to embed
 * does not make it legal.
 *
 * This code was written by Tom Murphy 7, and is public domain. Use at your
 * own risk...
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatal();

int main (int argc, char**argv) {
     FILE * inways;
     if (argc != 2)
        printf("Usage: %s font.ttf\n\nPublic Domain software by Tom 7. Use at your own risk.\n",argv[0]);
     else if (inways = fopen(argv[1],"rb+")) {
        int a,x;
        char type[5];
        type[4]=0;
        fseek(inways,12,0);
        for (;;) {
           for (x=0;x<4;x++) if (EOF == (type[x] = getc(inways))) fatal();
           if (!strcmp(type,"OS/2")) {
              int length;
              unsigned long loc, fstype, sum=0;
              loc=ftell(inways); /* location for checksum */
              for (x=4;x--;) if (EOF == getc(inways)) fatal();
              fstype  = fgetc(inways) << 24;
              fstype |= fgetc(inways) << 16;
              fstype |= fgetc(inways) << 8 ;
              fstype |= fgetc(inways)      ;
              length  = fgetc(inways) << 24;
              length |= fgetc(inways) << 16;
              length |= fgetc(inways) << 8 ;
              length |= fgetc(inways)      ;
/*              printf("fstype: %d length: %d\n",fstype,length);*/
              if (fseek(inways,fstype+8,0)) fatal();
              fputc(0,inways);
              fputc(0,inways);
              fseek(inways,fstype,0);
              for (x=length;x--;)
                  sum += fgetc(inways);
              fseek(inways,loc,0); /* write checksum */
              fputc(sum>>24,inways);
              fputc(255&(sum>>16),inways);
              fputc(255&(sum>>8), inways);
              fputc(255&sum    ,  inways);
              fclose(inways);
              exit(0);
           }
           for (x=12;x--;) if (EOF == getc(inways)) fatal();
        }

     } else
        printf("I wasn't able to open the file %s.\n", argv[1]);
}

void fatal() { fprintf(stderr,"Malformed TTF file.\n");
               exit(-1); }

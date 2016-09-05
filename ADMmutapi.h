/*
   mutant header file.
*/
#define JMAX 8   /* max junks allowed between working instructions, tune this for your exploit
                    CISC (3->10), RISC's keep (2-4) */
#define ALTERNATE 8 /* max alternate instruction path per instruction */
#define WINT 32   /* LCD width of an int, used for BCD calculations */

/* I try to use instruction set's that are widely supported, no +v8 sparc etc... */
#define DISABLE   0
#define IA32      1
#define SPARC     2
#define HPPA      3
#define MIPS      4
#define IA32_SLIDE 5

#define IA32_JUNKS 78
#define IA32_DECINST 13
#define IA32_DECINST_SLIDE IA32_DECINST+1

#define SPARC_JUNKS  21
#define SPARC_DECINST 15

#define HPPA_JUNKS 11
#define HPPA_DECINST 15

struct morphctl {
   int lower;           /* set for tolower resilelient code    */
   int upper;           /* set for toupper resilelient code    */
   unsigned int cipher; /* placeholder for 16bit key           */
   unsigned int cipher2;/* 2'nd placeholder for more keyspace  */
   unsigned int slide;  /* slide for cipher                    */
   unsigned int slide2; /* slide for cipher2                   */
   int keyslide;        /* toggle key sliding behaviour        */
   int arch;            /* arch selection                      */
   int junk_pads;       /* set by init_mutate                  */
   int decoder_instructions;  /* set by init_mutate            */
   char *banned;        /* banned list                         */
};

struct junks {
   char *code;          /* the operation code */
   size_t len;          /* length of code */
   int noppad;          /* can we use it for NOP pad? */
   int morph;           /* can we use it for morph pad? */
   int upper;           /* can we use through toupper()? */
   int lower;           /* can we use through tolower()? */
   int dyn;             /* is the instruction dynamic? */
   int dynint;          /* where is the dynamic byte? */
   int weight;          /* weight of this code to appear in output 1 == low, > 1 is higher :) */
};

struct decoder {
   int amount;          /* how many possiable instructions */
   int order;           /* BCD of whitch positions possiable, (11 == first and second possiable, 101 == third and first) */
   size_t lentohere;    /* a place to store our position */
   int upper;           /* # of instructions from the LEFT that can be used in toupper code */
   int lower;           /* # of instructions from the RIGHT that can be used in tolower code */
   int padd;            /* are we allowed to pad a NON-OP instruction after this one? */
   char *codez[ALTERNATE];
};

/* global pointers */
struct decoder *morpheng;
struct junks *njunk;

/* exported functions */
int init_mutate(struct morphctl *mut);
int apply_key(char *mutant, size_t enc_amount, size_t njlen, struct morphctl *mut);
int apply_engine(char *mutant, size_t enc_amount, size_t njlen, struct morphctl mut);
int apply_jnops(char *mutant, size_t njlen, struct morphctl mut);
int apply_offset_mod(char *mutant, size_t olen, size_t offoff, struct morphctl mut);

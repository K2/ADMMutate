/* local vulnerability */
#include <stdio.h>
#include <strings.h>
int doit(char *str)
{
    char bufz[1024];
    printf("doing stuffz...\n");
    strcpy(bufz,str);
    return(0);
}
int main(int argc, char **argv)
{
   printf("args %d\n",argc);
   if(argc>1)
   doit(argv[1]);
   return(0);
}

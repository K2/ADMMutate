/* remote vulnerability */
/* put me in inetd or something */
#include <stdio.h>
#if defined(LINUX) || defined(OS5) || defined(TRU64) || defined(BSD)
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>

int doit(char *str)
{
    char bufz[1024];
    printf("doing stuffz...\n");
    strcpy(bufz,str);
    return(0);
}
int main(int argc, char **argv)
{
   char buf[4096];
   int i=0;

   gets(buf);

   if(argc > 1) {
      if(!strcmp(argv[1],"lower")) {
         fprintf(stderr,"lower\n"); 
         while(buf[i]) {
            buf[i] = tolower(buf[i]);
            i++;
         }
      }
      if(!strcmp(argv[1],"upper")) {
         fprintf(stderr,"upper\n");     
         while(buf[i]) {
            buf[i] = toupper(buf[i]);
            i++;
         }
      }
   }
   doit(buf);
   printf("DONE STUFFZ [%s]\n",buf);
   return(0);
}

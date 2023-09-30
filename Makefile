# Pretty basic config here, 
# Uncomment the OS= for your OS (if it is defnined here)
# modify CFLAGS for high-resolution srand seed values
# if your not on IA32 arch,
# copy the exp*`arch`.c files to exp*.c (eg. cp exp-sparc.c exp.c, cp expx-sparc.c expx.c) TAKE THAT AUTOCONF@!!!#$()$@@

CC=gcc
CFLAGS=-O
CFLAGS=-O -DDSRAND -DIA32_RND
#CFLAGS=-O -DDSRAND -DSPARCV9_RND
#CFLAGS=-O -DDSRAND -DHPPA_RND
LFLAGS=-c
DEBUG=-DDEBUG -g3 -ggdb -DDEBUG_MORE

# If your compiling on one of these OS, uncomment it!!
OS=-DLINUX
#OS=-DHPUX
#OS=-DBSD
#OS=-DOS5
#OS=-DTRU64

all: ADMmuteng.o exp vulnerable m7 vulnerable-remote expx qp

ADMmuteng.o: ADMmuteng.c ADMmuteng.h ADMmutapi.h
	$(CC) $(DEBUG) $(CFLAGS) $(LFLAGS) -o ADMmuteng.o ADMmuteng.c $(OS)

m7: m7.c ADMmuteng.o
	$(CC) $(DEBUG) $(CFLAGS) -o m7 m7.c ADMmuteng.o $(OS)

exp: exp.c
	$(CC) $(DEBUG) -o exp exp.c $(OS)

expx: expx.c ADMmuteng.o
	$(CC) $(DEBUG) -o expx expx.c ADMmuteng.o $(OS)

vulnerable: vulnerable.c
	$(CC) $(DEBUG) -o vulnerable vulnerable.c $(OS)

vulnerable-remote: vulnerable-remote.c
	$(CC) $(DEBUG) -o vulnerable-remote vulnerable-remote.c $(OS)

qp: qp.c ADMmuteng.o
	$(CC) $(DEBUG) -o qp qp.c ADMmuteng.o

clean:
	rm -f m7 exp vulnerable core *.o zdec vulnerable-remote expx qp

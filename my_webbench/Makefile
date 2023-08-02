CFLAGS?= -Wall -ggdb -W -O
CC?=gcc
LIBS?=
LDFLAGS?=
PREFIX?=/usr/local/ 
VERSION=1.0
TMPDIR=/tmp/webbench-$(VERSION)

all: webbench tags

tags: *.c 
	-ctags *.c 

install:webbench
	#what is the install cmd mean?
	#this install program copies files (often just compiled) into destination loacations you choose. 
	#If you want to download and install a ready-to-use package on a GNU/Linux system, you should instead be using a package manager like apt-get
	# -d treat all arguments as directory names; create all components of the specified directories
  # -s strip symbol tables
	# -m set permission mode instead of rwxr-xr-x
	install -s webbench $(DESTDIR)$(PREFIX)/bin
	install -m 644 webbench.1 $(DESTDIR)$(PREFIX)/man/man1
	install -d $(DESTDIR)$(PREFIX)/share/doc/webbench
	install -m 644 debian/copyright $(DESTDIR)$(PREFIX)/share/doc/webbench
	install -m 644 debian/changelog $(DESTDIR)$(PREFIX)/share/doc/webbench

webbench: webbench.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o webbench webbench.o $(LIBS)

clean:
	-rm -f *.o webbench *~ core *.core tags



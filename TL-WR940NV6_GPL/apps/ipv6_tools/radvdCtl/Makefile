# $Id: Makefile,v 1.40 2006/02/13 03:06:25 quozl Exp $
VERSION=0.9.0
RELEASE=

CC = mips-linux-gcc
RM	= rm -f
CFLAGS  = -Wall -Wno-trigraphs -O2 -fomit-frame-pointer

#TP_WEB_LOG=y

#ifdef TP_WEB_LOG
#CFLAGS += -DTP_WEB_LOG
#endif

#ifdef TP_WEB_LOG
#LDFLAGS += -lmsglog
#endif

#added by ZQQ, 10.10.09
ifdef TP_WEB_LOG
CFLAGS  += -I ../../../web_server/msglogd/ -DTP_WEB_LOG
LDFLAGS += -L ../../../web_server/msglogd/ -lmsglog
endif
#end 

RADVDCTL = radvdctl

RADVDCTL_OBJS = radvdCtl.o 

all: $(RADVDCTL)

$(RADVDCTL): $(RADVDCTL_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(RADVDCTL) $(RADVDCTL_OBJS)

clean:
	$(RM) *.o $(RADVDCTL) 


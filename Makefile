# Makefile
# 2018-10-26  Markku-Juhani O. Saarinen <mjos@pqshield.com>
# (c) 2018 PQShield Ltd. All rights reserved.

DIST	= xef_ref
BIN	= xrun
OBJS	= xef_ref.o xef_test.o xe5_c64.o xe2_c16.o xe4_c64.o

CC	= gcc
CFLAGS	= -Wall -Ofast -march=native -std=c99
LIBS	= -lm
LDFLAGS	=
INCS	=

$(BIN):	$(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -f $(DIST)-*.t?z $(OBJS) $(BIN) *~

dist:	clean
	cd ..; \
	tar cfvz $(DIST)/$(DIST)-`date "+%Y%m%d%H%M"`.tgz $(DIST)/*

all: pwnbadge 

CFLAGS	= -Wall -g -O3 -Wno-unused-variable 
LDFLAGS	= -lm -lrt -lpthread 
CCP = g++ 
CC = gcc 

pwnbadge: pwnbadge.cpp src/librpitx.a
	$(CCP) $(CFLAGS) -o pwnbadge pwnbadge.cpp src/librpitx.a $(LDFLAGS) 

src/librpitx.a:
	$(MAKE) -C src

clean:
	rm -f pwnbadge
	$(MAKE) -C src clean

## Install any dependencies to the /include directory
IDIR=include
## Object files (.o) will be created here
ODIR=obj
## Our header files should be placed in the /lib directory
LDIR=lib
## Our source code should be in the /src directory
SRCDIR=src

CC=g++
CFLAGS=-I$(IDIR) -std=c++14 -I /usr/include -I/usr/local/include -L /usr/lib -L/usr/local/lib -lwiringPi -pthread -lssl -lcrypto -lboost_system

LIBS=-lm

_DEPS = datasource.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = socketServer.o max30100Datasource.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(OBJ): $(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

simpleDataCollector: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

socketServer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

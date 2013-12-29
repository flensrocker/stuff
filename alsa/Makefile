
CFLAGS += $(shell pkg-config --cflags alsa jsoncpp)
LIBS   += $(shell pkg-config --libs alsa jsoncpp)

all: getsoundcardbyname listsoundcardids

%.o: %.c
	gcc -c $^ $(CFLAGS)

%.o: %.cpp
	g++ -c $^ $(CFLAGS)

getsoundcardbyname: getsoundcardbyname.o
	gcc -o $@ $< $(LIBS)

listsoundcardids: listsoundcardids.o
	g++ -o $@ $< $(LIBS)

clean:
	rm -f *.o getsoundcardbyname listsoundcardids


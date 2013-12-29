
all: getsoundcardbyname listsoundcardids

%.o: %.c
	gcc -c $^ $(shell pkg-config --cflags alsa)

getsoundcardbyname: getsoundcardbyname.o
	gcc -o $@ $< $(shell pkg-config --libs alsa)

listsoundcardids: listsoundcardids.o
	gcc -o $@ $< $(shell pkg-config --libs alsa)

clean:
	rm -f *.o getsoundcardbyname listsoundcardids


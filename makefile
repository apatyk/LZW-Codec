CC = gcc
CFLAGS = -Wall

BINS = lzw

all: $(BINS)

$(BINS):  $(BINS).c
	$(CC) -o $(BINS) $(BINS).c $(CFLAGS)

style:
	astyle --style=java --break-blocks --pad-oper --pad-header --align-pointer=name --delete-empty-lines *.c

clean:
	rm $(BINS)
	rm *.lzw
	rm *-recovered*

cleano:
	rm *.orig

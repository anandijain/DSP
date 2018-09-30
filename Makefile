CC = -gcc
CFLAGS = -Wall -lm -o

wavgen: wavgen.c
	$(CC) $(CFLAGS) wavgen wavgen.c wavgen.h

clean:
	rm -rf *.o wavgen
	find . -name "*.wav" -type f -delete

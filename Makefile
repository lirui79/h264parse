
all:h264parse

h264parse:h264parse.c  header.c nalutil.c nalu.c  parset.c  slice.c  stream.c
	gcc -g h264parse.c  header.c nalutil.c nalu.c  parset.c  slice.c  stream.c -I./ -o h264parse

clean:
	rm h264parse *.o
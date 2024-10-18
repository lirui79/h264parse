
all:h264a

h264a:decode.c  header.c  nalu.c  parset.c  slice.c  stream.c
	gcc -g decode.c  header.c  nalu.c  parset.c  slice.c  stream.c -I./ -o h264a
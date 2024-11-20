
all:h264parse

h264parse:h264parse.c avc_sps.c avc_pps.c nalutil.c nalu.c avc_slice.c datafile.c avc_stream.c
	gcc -g h264parse.c avc_sps.c avc_pps.c nalutil.c nalu.c avc_slice.c datafile.c avc_stream.c -I./ -o h264parse

clean:
	rm h264parse *.o
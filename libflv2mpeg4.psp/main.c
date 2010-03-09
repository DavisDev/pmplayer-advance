#include<stdio.h>
#include<stdlib.h>
#include "flv2mpeg4.h"

void flv2mpeg4(const char* flv) {
	
	char mpeg4[4096];
	memset(mpeg4, 0, 4096);
	sprintf(mpeg4, "%s.dat", flv);
	
	FILE* flv_fp = fopen(flv, "rb");
	fseek(flv_fp, 0, SEEK_END);
	int flv_buf_size = ftell(flv_fp);
	char* flv_buf = malloc(flv_buf_size);
	fseek(flv_fp, 0, SEEK_SET);
	fread(flv_buf, 1, flv_buf_size, flv_fp);
	fclose(flv_fp);
	flv_fp = 0;
	
	FILE* mpeg4_fp = fopen(mpeg4, "wb+");
	
	open_flv2mpeg4_convert(320, 240);
	
	int outbuf_size;
	void* outbuf = convert_flv_frame_to_mpeg4_frame(flv_buf, flv_buf_size, &outbuf_size);
	
	if ( outbuf )
		fwrite(outbuf, outbuf_size, 1, mpeg4_fp);
	
	fclose(mpeg4_fp);
	mpeg4_fp = 0;
	
	close_flv2mpeg4_convert();
	
}


int main(int argc, char** argv) {
	
	if ( argc < 2 ) {
		//usage();
	}
	else {
		flv2mpeg4(argv[1]);
	}
	return 0;
}
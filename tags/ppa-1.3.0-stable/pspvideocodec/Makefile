PSPSDK=$(shell psp-config --pspsdk-path)
AUDIODRIVER_OBJS = sceVideocodec_0000.o sceVideocodec_0001.o sceVideocodec_0002.o sceVideocodec_0003.o sceVideocodec_0004.o sceVideocodec_0005.o sceVideocodec_0006.o sceVideocodec_0007.o sceVideocodec_0008.o sceVideocodec_0009.o sceVideocodec_0010.o

%.o: sceVideocodec.S
	psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(AUDIODRIVER_OBJS)
	psp-ar cru ../libpspvideocodec.a $(AUDIODRIVER_OBJS)
	psp-ranlib ../libpspvideocodec.a

clean:
	rm -f *.o
	rm -f ../libpspvideocodec.a

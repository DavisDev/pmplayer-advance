CFLAGS = -Wall -pedantic
CXXFLAGS = $(CFLAGS)
LDFLAGS =

all: amrnb-dec amrnb-enc amrwb-dec

amrnb-dec: amrnb-dec.o wav.o
	$(CXX) -o $@ $+ $(LDFLAGS) -lopencore-amrnb

amrnb-enc: amrnb-enc.o
	$(CC) -o $@ $+ $(LDFLAGS) -lm -lopencore-amrnb

amrwb-dec: amrwb-dec.o wav.o
	$(CXX) -o $@ $+ $(LDFLAGS) -lopencore-amrwb

# To check that both libraries can be linked in statically at the same
# time without duplicate symbols
linkboth: linkboth.o
	$(CC) -static -o $@ $+ $(LDFLAGS) -lopencore-amrnb -lopencore-amrwb

clean:
	rm -f amrnb-dec amrnb-enc amrwb-dec linkboth *.o out.wav out.amr


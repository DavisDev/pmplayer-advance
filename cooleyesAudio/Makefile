all:
	make -C src
	cp src/cooleyesAudio.prx .
	psp-build-exports -s src/cooleyesAudio.exp

clean:
	make -C src clean
	rm -f cooleyesAudio.prx
	rm -f cooleyesAudio.S


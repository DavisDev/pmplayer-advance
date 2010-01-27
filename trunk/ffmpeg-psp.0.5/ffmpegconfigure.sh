CFLAGS='-I.. -I/usr/local/pspdev/psp/sdk/include -g -O2 -G0' \
./configure --prefix=`pwd` --cross-prefix=psp- --enable-cross-compile --target-os=psp --arch=psp --enable-hardcoded-tables \
--enable-gpl --disable-vhook --enable-small \
--disable-demuxers --disable-muxers \
--disable-encoders --disable-decoders --disable-parsers --disable-protocols --disable-bsfs \
--enable-decoder=flv
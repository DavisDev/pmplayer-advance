all:
	rm -rf release/*
	make clean
	make ppa100zh
	make ppa100en
	make clean
	make ppa150zh
	make ppa150en
	make clean
	make ppa3xxzh
	make ppa3xxen
eboot100:
#	make -C libfaad2
	make -C libmpeg
	make -C pspvideocodec
#	make -C libavutil
#	make -C libavformat
#	make -C libavcodec
	make -C ppa
	mkdir -p ppa/PPA
	rm -rf ppa/PPA/*
	cp -f ppa/EBOOT.PBP ppa/PPA
	
eboot150:
#	make -C libfaad2
	make -C libmpeg
	make -C pspvideocodec
#	make -C libavutil
#	make -C libavformat
#	make -C libavcodec
	rm -rf ppa/__SCE__PPA/*
	make -C ppa ppa150
	
eboot3xx:
#	make -C libfaad2
	make -C libmpeg
	make -C pspvideocodec
	make -C cooleyesAudio
#	make -C libavutil
#	make -C libavformat
#	make -C libavcodec
	make -C ppa ppa3xx
	mkdir -p ppa/PPA3xx
	rm -rf ppa/PPA3xx/*
	cp -f ppa/EBOOT.PBP ppa/PPA3xx
	
ppa100zh: eboot100
	cp -f ppa/extra/*.prx ppa/PPA
	cp -f ppa/extra/config.xml ppa/PPA/config.xml
	mkdir -p ppa/PPA/fonts
	mkdir -p ppa/PPA/skins/default
	mkdir -p ppa/PPA/skins/normal-en
	mkdir -p ppa/PPA/skins/MHP
	cp -f ppa/extra/fonts/*.* ppa/PPA/fonts
	cp -f ppa/extra/skins/default/*.* ppa/PPA/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/PPA/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/PPA/skins/MHP
	rar a -ep1 release/ppa.eboot.fw100.`date +"%Y%m%d"`.rar ppa/PPA/EBOOT.PBP
	rar a -ep1 release/ppa.eboot.fw100.`date +"%Y%m%d"`.rar ppa/PPA/usbhostfs.prx
	rar a -ep1 release/ppa.eboot.fw100.`date +"%Y%m%d"`.rar ppa/PPA/netuser.prx
	rar a -ep1 release/ppa.eboot.fw100.`date +"%Y%m%d"`.rar ppa/PPA/nethostfs.prx
	rar a -ep1 release/ppa.all-zh.fw100.`date +"%Y%m%d"`.rar ppa/PPA
	cat cngba.txt | rar c release/ppa.eboot.fw100.`date +"%Y%m%d"`.rar
	cat cngba.txt | rar c release/ppa.all-zh.fw100.`date +"%Y%m%d"`.rar
	
ppa100en: eboot100
	cp -f ppa/extra/*.prx ppa/PPA
	cp -f ppa/extra/config-en.xml ppa/PPA/config.xml
	mkdir -p ppa/PPA/fonts
	mkdir -p ppa/PPA/skins/default
	mkdir -p ppa/PPA/skins/normal-en
	mkdir -p ppa/PPA/skins/MHP
	cp -f ppa/extra/fonts/wqy-bsong.ttf ppa/PPA/fonts
	cp -f ppa/extra/skins/default/*.* ppa/PPA/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/PPA/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/PPA/skins/MHP
	rar a -ep1 release/ppa.all-en.fw100.`date +"%Y%m%d"`.rar ppa/PPA
	cat cngba.txt | rar c release/ppa.all-en.fw100.`date +"%Y%m%d"`.rar
	
ppa150zh: eboot150
	cp -f ppa/extra/*.prx ppa/__SCE__PPA
	cp -f ppa/extra/config.xml ppa/__SCE__PPA/config.xml
	mkdir -p ppa/__SCE__PPA/fonts
	mkdir -p ppa/__SCE__PPA/skins/default
	mkdir -p ppa/__SCE__PPA/skins/normal-en
	mkdir -p ppa/__SCE__PPA/skins/MHP
	cp -f ppa/extra/fonts/*.* ppa/__SCE__PPA/fonts
	cp -f ppa/extra/skins/default/*.* ppa/__SCE__PPA/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/__SCE__PPA/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/__SCE__PPA/skins/MHP
	rar a -ep1 release/ppa.eboot.fw150.`date +"%Y%m%d"`.rar ppa/__SCE__PPA/EBOOT.PBP
	rar a -ep1 release/ppa.eboot.fw150.`date +"%Y%m%d"`.rar ppa/__SCE__PPA/usbhostfs.prx
	rar a -ep1 release/ppa.eboot.fw150.`date +"%Y%m%d"`.rar ppa/__SCE__PPA/netuser.prx
	rar a -ep1 release/ppa.eboot.fw150.`date +"%Y%m%d"`.rar ppa/__SCE__PPA/nethostfs.prx
	rar a -ep1 release/ppa.all-zh.fw150.`date +"%Y%m%d"`.rar ppa/__SCE__PPA ppa/%__SCE__PPA
	cat cngba.txt | rar c release/ppa.eboot.fw150.`date +"%Y%m%d"`.rar
	cat cngba.txt | rar c release/ppa.all-zh.fw150.`date +"%Y%m%d"`.rar
	
ppa150en: eboot150
	cp -f ppa/extra/*.prx ppa/__SCE__PPA
	cp -f ppa/extra/config-en.xml ppa/__SCE__PPA/config.xml
	mkdir -p ppa/__SCE__PPA/fonts
	mkdir -p ppa/__SCE__PPA/skins/default
	mkdir -p ppa/__SCE__PPA/skins/normal-en
	mkdir -p ppa/__SCE__PPA/skins/MHP
	cp -f ppa/extra/fonts/wqy-bsong.ttf ppa/__SCE__PPA/fonts
	cp -f ppa/extra/skins/default/*.* ppa/__SCE__PPA/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/__SCE__PPA/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/__SCE__PPA/skins/MHP
	rar a -ep1 release/ppa.all-en.fw150.`date +"%Y%m%d"`.rar ppa/__SCE__PPA ppa/%__SCE__PPA
	cat cngba.txt | rar c release/ppa.all-en.fw150.`date +"%Y%m%d"`.rar

ppa3xxzh: eboot3xx
	cp -f ppa/extra/*.prx ppa/PPA3xx
	cp -f ppa/extra/config.xml ppa/PPA3xx/config.xml
	mkdir -p ppa/PPA3xx/fonts
	mkdir -p ppa/PPA3xx/skins/default
	mkdir -p ppa/PPA3xx/skins/normal-en
	mkdir -p ppa/PPA3xx/skins/MHP
	cp -f cooleyesAudio/cooleyesAudio.prx ppa/PPA3xx
	cp -f ppa/extra/fonts/*.* ppa/PPA3xx/fonts
	cp -f ppa/extra/skins/default/*.* ppa/PPA3xx/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/PPA3xx/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/PPA3xx/skins/MHP
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/EBOOT.PBP 
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/cooleyesAudio.prx 
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/usbhostfs.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/netuser.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/netuser_302.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/nethostfs.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/nethostfs_302.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/nethostfs_310.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/nethostfs_371.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/dvemgr.prx
	rar a -ep1 release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx/config.xml
	rar a -ep1 release/ppa.all-zh.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx
	cat cngba.txt | rar c release/ppa.eboot.fw3xx.`date +"%Y%m%d"`.rar
	cat cngba.txt | rar c release/ppa.all-zh.fw3xx.`date +"%Y%m%d"`.rar
	
ppa3xxen: eboot3xx
	cp -f ppa/extra/*.prx ppa/PPA3xx
	cp -f ppa/extra/config-en.xml ppa/PPA3xx/config.xml
	mkdir -p ppa/PPA3xx/fonts
	mkdir -p ppa/PPA3xx/skins/default
	mkdir -p ppa/PPA3xx/skins/normal-en
	mkdir -p ppa/PPA3xx/skins/MHP
	cp -f cooleyesAudio/cooleyesAudio.prx ppa/PPA3xx
	cp -f ppa/extra/fonts/wqy-bsong.ttf ppa/PPA3xx/fonts
	cp -f ppa/extra/skins/default/*.* ppa/PPA3xx/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/PPA3xx/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/PPA3xx/skins/MHP
	rar a -ep1 release/ppa.all-en.fw3xx.`date +"%Y%m%d"`.rar ppa/PPA3xx
	cat cngba.txt | rar c release/ppa.all-en.fw3xx.`date +"%Y%m%d"`.rar

clean:
#	make -C libfaad2 clean
	make -C libmpeg clean
	make -C pspvideocodec clean
	make -C cooleyesAudio clean
#	make -C libavutil clean
#	make -C libavformat clean
#	make -C libavcodec clean
	make -C ppa clean

all:
	rm -rf release/*
	make clean
	make ppa3xxzh
	make ppa3xxen
	make miniconv-euc
	make miniconv-cjk
	make i18n-eng
	make i18n-chs
	make i18n-cht
	
eboot3xx:
	make -C libmpeg
	make -C pspvideocodec
	make -C cooleyesBridge
	make -C libbufferedio.psp
	make -C libmp4info.psp
	make -C libmkvinfo.psp
	make -C libflv1info.psp
	make -C opencore-amr.psp
	make -C ppa/miniconv
	make -C ppa/i18n
	make -C ppa ppa3xx
	mkdir -p ppa/PPA3xx
	rm -rf ppa/PPA3xx/*
	cp -f ppa/EBOOT.PBP ppa/PPA3xx
	
ppa3xxzh: eboot3xx
	cp -f ppa/extra/*.prx ppa/PPA3xx
	cp -f ppa/extra/config.xml ppa/PPA3xx/config.xml
	mkdir -p ppa/PPA3xx/fonts
	mkdir -p ppa/PPA3xx/skins/default
	mkdir -p ppa/PPA3xx/skins/normal-en
	mkdir -p ppa/PPA3xx/skins/MHP
	cp -f cooleyesBridge/cooleyesBridge.prx ppa/PPA3xx
	cp -f ppa/miniconv/miniconv.cjk.prx ppa/PPA3xx/miniconv.prx
	cp -f ppa/i18n/i18n.chs.prx ppa/PPA3xx/i18n.prx
	cp -f ppa/extra/fonts/*.* ppa/PPA3xx/fonts
	cp -f ppa/extra/skins/default/*.* ppa/PPA3xx/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/PPA3xx/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/PPA3xx/skins/MHP
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/EBOOT.PBP 
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/cooleyesBridge.prx 
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/usbhostfs.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/netuser.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/netuser_302.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs_302.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs_310.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs_371.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/dvemgr.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/mpeg_vsh330.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/mpeg_vsh350.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/mpeg_vsh370.prx
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/miniconv.prx 
	rar a -ep1 release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/i18n.prx 
	rar a -ep1 release/ppa.all.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx
	cat cngba.txt | rar c release/ppa.eboot.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar
	cat cngba.txt | rar c release/ppa.all.chs.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar
	
ppa3xxen: eboot3xx
	cp -f ppa/extra/*.prx ppa/PPA3xx
	cp -f ppa/extra/config-en.xml ppa/PPA3xx/config.xml
	mkdir -p ppa/PPA3xx/fonts
	mkdir -p ppa/PPA3xx/skins/default
	mkdir -p ppa/PPA3xx/skins/normal-en
	mkdir -p ppa/PPA3xx/skins/MHP
	cp -f cooleyesBridge/cooleyesBridge.prx ppa/PPA3xx
	cp -f ppa/miniconv/miniconv.eur.prx ppa/PPA3xx/miniconv.prx
	cp -f ppa/i18n/i18n.eng.prx ppa/PPA3xx/i18n.prx
	cp -f ppa/extra/fonts/*.* ppa/PPA3xx/fonts
	cp -f ppa/extra/skins/default/*.* ppa/PPA3xx/skins/default
	cp -f ppa/extra/skins/normal-en/*.* ppa/PPA3xx/skins/normal-en
	cp -f ppa/extra/skins/MHP/*.* ppa/PPA3xx/skins/MHP
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/EBOOT.PBP 
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/cooleyesBridge.prx 
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/usbhostfs.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/netuser.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/netuser_302.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs_302.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs_310.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/nethostfs_371.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/dvemgr.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/mpeg_vsh330.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/mpeg_vsh350.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/mpeg_vsh370.prx
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/miniconv.prx 
	rar a -ep1 release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx/i18n.prx 
	rar a -ep1 release/ppa.all.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar ppa/PPA3xx
	cat cngba.txt | rar c release/ppa.eboot.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar
	cat cngba.txt | rar c release/ppa.all.en.fw3xx.`tail ppa/version.txt`\(`date +"%Y%m%d"`\).rar
	
miniconv-euc:
	cp -f ppa/miniconv/miniconv.eur.prx /tmp/miniconv.prx
	rar a -ep1 release/miniconv.eur.prx.`date +"%Y%m%d"`.rar /tmp/miniconv.prx
	cat cngba.txt | rar c release/miniconv.eur.prx.`date +"%Y%m%d"`.rar

miniconv-cjk:
	cp -f ppa/miniconv/miniconv.cjk.prx /tmp/miniconv.prx
	rar a -ep1 release/miniconv.cjk.prx.`date +"%Y%m%d"`.rar /tmp/miniconv.prx
	cat cngba.txt | rar c release/miniconv.cjk.prx.`date +"%Y%m%d"`.rar	

i18n-eng:
	cp -f ppa/i18n/i18n.eng.prx /tmp/i18n.prx
	rar a -ep1 release/i18n.eng.prx.`date +"%Y%m%d"`.rar /tmp/i18n.prx
	cat cngba.txt | rar c release/i18n.eng.prx.`date +"%Y%m%d"`.rar

i18n-chs:
	cp -f ppa/i18n/i18n.chs.prx /tmp/i18n.prx
	rar a -ep1 release/i18n.chs.prx.`date +"%Y%m%d"`.rar /tmp/i18n.prx
	cat cngba.txt | rar c release/i18n.chs.prx.`date +"%Y%m%d"`.rar
	
i18n-cht:
	cp -f ppa/i18n/i18n.cht.prx /tmp/i18n.prx
	rar a -ep1 release/i18n.cht.prx.`date +"%Y%m%d"`.rar /tmp/i18n.prx
	cat cngba.txt | rar c release/i18n.cht.prx.`date +"%Y%m%d"`.rar

clean:
	make -C libmpeg clean
	make -C pspvideocodec clean
	make -C cooleyesBridge clean
	make -C libbufferedio.psp clean
	make -C libmp4info.psp clean
	make -C libmkvinfo.psp clean
	make -C libflv1info.psp clean
	make -C opencore-amr.psp clean
	make -C ppa clean
	rm -f *.bak

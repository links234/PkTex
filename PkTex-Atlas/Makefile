NPROC := $(shell grep -c ^processor /proc/cpuinfo)

install:
	sudo -E sh scripts/config_linux.sh
	make clean -f "jni/Linux.mk"
	make -j$(NPROC) -f "jni/Linux.mk"
	sudo cp pktex-atlas /usr/bin/pktex-atlas

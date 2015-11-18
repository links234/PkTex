install:
	@echo "Installing PkTex tools ..."
	@echo "Installing pktex-resize ..."
	cd PkTex-Resize
	make install -f "PkTex-Resize/Makefile"
	cd ..
	cp PkTex-Resize/pktex-resize.py /usr/bin/pktex-resize
	@echo "pktex-resize install done!"
	@echo "PkTex tools install completed!"

install:
	@echo "Installing PkTex tools ..."

	@echo "Installing pktex-resize ..."
	make -C PkTex-Resize install
	@echo "pktex-resize install done!"

	@echo "Installing pktex-atlas ..."
	make -C PkTex-Atlas install
	@echo "pktex-atlas install done!"

	@echo "Installing pktex-meta ..."
	make -C PkTex-Meta install
	@echo "pktex-resize install done!"

	@echo "PkTex tools install completed!"

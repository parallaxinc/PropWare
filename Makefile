all:
	$(MAKE) -C cmm
	$(MAKE) -C lmm
	$(MAKE) -C xmm
	$(MAKE) -C xmmc
	$(MAKE) -C PropGCC_Demos
	
clean:
	$(MAKE) -C cmm clean
	$(MAKE) -C lmm clean
	$(MAKE) -C xmm clean
	$(MAKE) -C xmmc clean
	$(MAKE) -C PropGCC_Demos clean
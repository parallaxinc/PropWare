all:
	$(MAKE) -C cmm
	$(MAKE) -C lmm
	$(MAKE) -C xmm
	
clean:
	$(MAKE) -C cmm clean
	$(MAKE) -C lmm clean
	$(MAKE) -C xmm clean
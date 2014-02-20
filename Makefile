all:
	$(MAKE) -C simple
	$(MAKE) -C cmm
	$(MAKE) -C lmm
	$(MAKE) -C xmm
	$(MAKE) -C xmmc
	$(MAKE) -C Examples

clean:
	$(MAKE) -C cmm clean
	$(MAKE) -C lmm clean
	$(MAKE) -C xmm clean
	$(MAKE) -C xmmc clean
	$(MAKE) -C Examples clean

simple_clean:
	$(MAKE) -C simple clean
all:
	$(MAKE) -C simple
	$(MAKE) -C PropWare
	$(MAKE) -C Examples

clean:
	$(MAKE) -C PropWare clean
	$(MAKE) -C Examples clean

simple_clean:
	$(MAKE) -C simple clean
all:
	$(shell test -d HD44780/Debug || mkdir -p HD44780/Debug)
	$(shell test -d L3G/Debug || mkdir -p L3G/Debug)
	$(shell test -d MAX6675/Debug || mkdir -p MAX6675/Debug)
	$(shell test -d MCP300x/Debug || mkdir -p MCP300x/Debug)
	$(shell test -d SD/Debug || mkdir -p SD/Debug)
	$(shell test -d SPI/Debug || mkdir -p SPI/Debug)
	$(MAKE) -C HD44780/Debug -f ../Makefile
	$(MAKE) -C L3G/Debug -f ../Makefile
	$(MAKE) -C MAX6675/Debug -f ../Makefile
	$(MAKE) -C MCP300x/Debug -f ../Makefile
	$(MAKE) -C SD/Debug -f ../Makefile
	$(MAKE) -C SPI/Debug -f ../Makefile

clean:
	$(shell test -d HD44780/Debug || mkdir -p HD44780/Debug)
	$(shell test -d L3G/Debug || mkdir -p L3G/Debug)
	$(shell test -d MAX6675/Debug || mkdir -p MAX6675/Debug)
	$(shell test -d MCP300x/Debug || mkdir -p MCP300x/Debug)
	$(shell test -d SD/Debug || mkdir -p SD/Debug)
	$(shell test -d SPI/Debug || mkdir -p SPI/Debug)
	$(MAKE) -C HD44780/Debug -f ../Makefile clean
	$(MAKE) -C L3G/Debug -f ../Makefile clean
	$(MAKE) -C MAX6675/Debug -f ../Makefile clean
	$(MAKE) -C MCP300x/Debug -f ../Makefile clean
	$(MAKE) -C SD/Debug -f ../Makefile clean
	$(MAKE) -C SPI/Debug -f ../Makefile clean

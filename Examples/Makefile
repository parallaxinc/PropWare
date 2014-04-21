ifeq ($(OS), Windows_NT)
	DIR_DIV=\\\\
# When the Windows command mkdir tries to create a directory that already
# exists, an error is thrown. Though this does not cause any problems for Make,
# it does clutter up the terminal significantly. These "errors" can be safely
# ignored and thrown into nul
	NULL=2> nul
else
	MKDIR_FLAG=-p
	DIR_DIV=/
endif

all:
	$(shell mkdir $(MKDIR_FLAG) PropWare_HD44780$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_HD44780/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_L3G$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_L3G/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_MAX6675$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_MAX6675/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_MCP3000$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_MCP3000/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_SD$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_SD/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_SPI$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_SPI/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_UART$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_UART/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare_I2C_HD44780$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Concurrency/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) PropWare$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) Simple_I2C$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Simple_I2C/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) Simple_SimpleText$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Simple_SimpleText/Debug -f ../Makefile

	$(shell mkdir $(MKDIR_FLAG) Concurrency$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Concurrency/Debug -f ../Makefile

clean:
	$(shell mkdir $(MKDIR_FLAG) PropWare_HD44780$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_HD44780/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_L3G$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_L3G/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_MAX6675$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_MAX6675/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_MCP3000$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_MCP3000/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_SD$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_SD/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_SPI$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_SPI/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_UART$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare_UART/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare_I2C_HD44780$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Concurrency/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) PropWare$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) Simple_I2C$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Simple_I2C/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) Simple_SimpleText$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Simple_SimpleText/Debug -f ../Makefile clean

	$(shell mkdir $(MKDIR_FLAG) Concurrency$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C Concurrency/Debug -f ../Makefile clean


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
	$(shell mkdir $(MKDIR_FLAG) HD44780$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C HD44780/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) L3G$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C L3G/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) MAX6675$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C MAX6675/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) MCP300x$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C MCP300x/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) SD$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C SD/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) SPI$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C SPI/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) PropWare$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) I2C$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C I2C/Debug -f ../Makefile

clean:
	$(shell mkdir $(MKDIR_FLAG) HD44780$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C HD44780/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) L3G$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C L3G/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) MAX6675$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C MAX6675/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) MCP300x$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C MCP300x/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) SD$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C SD/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) SPI$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C SPI/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) PropWare$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C PropWare/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) I2C$(DIR_DIV)Debug $(NULL))
	$(MAKE) -C I2C/Debug -f ../Makefile clean

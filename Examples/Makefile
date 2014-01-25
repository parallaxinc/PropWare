ifeq ($(OS), Windows_NT)
	DIR_DIV=\\\\
else
	MKDIR_FLAG=-p
	DIR_DIV=/
endif

all:
	$(shell mkdir $(MKDIR_FLAG) HD44780$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C HD44780/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) L3G$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C L3G/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) MAX6675$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C MAX6675/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) MCP300x$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C MCP300x/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) SD$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C SD/Debug -f ../Makefile
	
	$(shell mkdir $(MKDIR_FLAG) SPI$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C SPI/Debug -f ../Makefile

clean:
	$(shell mkdir $(MKDIR_FLAG) HD44780$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C HD44780/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) L3G$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C L3G/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) MAX6675$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C MAX6675/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) MCP300x$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C MCP300x/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) SD$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C SD/Debug -f ../Makefile clean
	
	$(shell mkdir $(MKDIR_FLAG) SPI$(DIR_DIV)Debug > nul 2> nul)
	$(MAKE) -C SPI/Debug -f ../Makefile clean

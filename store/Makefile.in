all:

%:
	@$(MAKE) -C .. $*


test:
	 $(MAIN)
	./schema-setup.sh
	LD_LIBRARY_PATH=. ./upk
	./sqlite-dump

LIBS_INC=lib/include
LIBS_SRCS=lib/src
LIBS_OUT=$(BUILD_FOLDER)/lib
.PHONY: libs-init libs-compile

libs-init: 
	mkdir -p $(LIBS_OUT)

libs-compile: libs-init 
	gcc -std=$(STD) -o $(LIBS_OUT)/tools.o -c $(LIBS_SRCS)/tools.c -I $(LIBS_INC)
	gcc -std=$(STD) -o $(LIBS_OUT)/data.o -c $(LIBS_SRCS)/data.c -I $(LIBS_INC)

	ln -s -f ../../$(LIBS_INC)/tools.h $(LIBS_OUT)/tools.h
	ln -s -f ../../$(LIBS_INC)/data.h $(LIBS_OUT)/data.h

libs-clean: 
	rm -rf $(LIBS_OUT)

TOOLS_SRC_FOLDER:=src
TOOLS_OUT_FOLDER:=out
TOOLS_SRC:=tools

.PHONY: libs-compile-tools libs-clean-tools

libs-compile-tools: libs-compile-data 
	mkdir -p $(LIBS_SRCS)/$(TOOLS_SRC)/$(TOOLS_OUT_FOLDER)
	
	gcc -std=c11 -o $(LIBS_SRCS)/$(TOOLS_SRC)/$(TOOLS_OUT_FOLDER)/$(TOOLS_SRC).o -c $(LIBS_SRCS)/$(TOOLS_SRC)/$(TOOLS_SRC_FOLDER)/$(TOOLS_SRC).c -I $(LIBS) 

	ln -s -f ../../$(LIBS_SRCS)/$(TOOLS_SRC)/$(TOOLS_OUT_FOLDER)/$(TOOLS_SRC).o $(LIBS)/$(TOOLS_SRC).o
	ln -s -f ../../$(LIBS_SRCS)/$(TOOLS_SRC)/$(TOOLS_SRC_FOLDER)/$(TOOLS_SRC).h $(LIBS)/$(TOOLS_SRC).h

libs-clean-tools:
	rm -rf $(TOOLS_OUT_FOLDER)
	

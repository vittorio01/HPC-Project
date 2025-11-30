DATA_SRC_FOLDER:=src
DATA_OUT_FOLDER:=out
DATA_SRC:=data

.PHONY: libs-compile-data libs-clean-data

libs-compile-data: 
	mkdir -p $(LIBS_SRCS)/$(DATA_SRC)/$(DATA_OUT_FOLDER)
	
	gcc -std=c11 -o $(LIBS_SRCS)/$(DATA_SRC)/$(DATA_OUT_FOLDER)/$(DATA_SRC).o -c $(LIBS_SRCS)/$(DATA_SRC)/$(DATA_SRC_FOLDER)/$(DATA_SRC).c -I $(LIBS) 

	ln -s -f ../../$(LIBS_SRCS)/$(DATA_SRC)/$(DATA_OUT_FOLDER)/$(DATA_SRC).o $(LIBS)/$(DATA_SRC).o
	ln -s -f ../../$(LIBS_SRCS)/$(DATA_SRC)/$(DATA_SRC_FOLDER)/$(DATA_SRC).h $(LIBS)/$(DATA_SRC).h

libs-clean-data:
	rm -rf $(DATA_OUT_FOLDER)
	

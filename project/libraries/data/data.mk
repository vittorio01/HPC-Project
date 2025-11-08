SRC_FOLDER=src
OUT_FOLDER=out
SRC=data

.PHONY: libs-compile libs-clean

libs-compile: 
	mkdir -p $(LIBS_SRCS)/$(SRC)/$(OUT_FOLDER)
	gcc -o $(LIBS_SRCS)/$(SRC)/$(OUT_FOLDER)/$(SRC).o -c $(LIBS_SRCS)/$(SRC)/$(SRC_FOLDER)/$(SRC).c
	ln -s $(LIBS_SRCS)/$(SRC)/$(OUT_FOLDER)/$(SRC).o $(LIBS)/$(SRC).o
	ln -s $(LIBS_SRCS)/$(SRC)/$(SRC_FOLDER)/$(SRC).h $(LIBS)/$(SRC).h

libs-clean:
	rm -rf $(OUT_FOLDER)
	rm $(LIBS)/$(SRC).o
	rm $(LIBS)/$(SRC).h
	

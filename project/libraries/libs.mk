LIBS=libraries/out
LIBS_SRCS=libraries
.PHONY: libs-init libs-compile

libs-init: 
	mkdir -p $(LIBS)

libs-compile: libs-clean libs-init

include libraries/data/data.mk

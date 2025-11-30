LIBS=libraries/out
LIBS_SRCS=libraries
.PHONY: libs-init libs-compile

libs-init: 
	mkdir -p $(LIBS)

libs-compile: libs-clean libs-init libs-compile-data libs-compile-tools

libs-clean: libs-clean-data libs-clean-tools

include libraries/tools/tools.mk
include libraries/data/data.mk

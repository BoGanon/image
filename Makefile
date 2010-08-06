EE_LIB_DIR = lib
EE_INC_DIR = include
EE_SRC_DIR = src

EE_LIB = $(EE_LIB_DIR)/libimage.a
EE_INCS = -I./include -I$(PS2SDK)/ports/include
EE_OBJS = $(EE_SRC_DIR)/image.o $(EE_SRC_DIR)/png_image.o
EE_LIBS = -lpng -lz -lmf

all: $(EE_LIB_DIR) $(EE_LIB)

$(EE_LIB_DIR) : 
	mkdir -p $(EE_LIB_DIR)

install: all
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/include
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/lib
	cp -f $(EE_LIB) $(DESTDIR)$(PS2SDK)/ports/lib
	cp -f $(EE_INC_DIR)/image.h $(DESTDIR)$(PS2SDK)/ports/include

clean:
	/bin/rm -rf $(EE_OBJS) $(EE_LIB) $(EE_LIB_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

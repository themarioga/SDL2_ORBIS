TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
PROJDIR     := $(shell basename $(CURDIR))
INTDIR      := bin
INCDIR		:= include
LIBNAME		:= lib$(PROJDIR).a

# Compiler options. You likely won't need to touch these.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
		CC      := clang++
		AR      := llvm-ar
		CDIR    := linux
endif
ifeq ($(UNAME_S),Darwin)
		CC      := /usr/local/opt/llvm/bin/clang++
		AR      := /usr/local/opt/llvm/bin/llvm-ar
		CDIR    := macos
endif
ODIR        := $(INTDIR)
SDIR        := source
IDIRS       := -I$(TOOLCHAIN)/include -I$(TOOLCHAIN)/include/c++/v1 -I$(INCDIR)
CFLAGS      := -cc1 -triple x86_64-pc-freebsd-elf -munwind-tables $(IDIRS) -fuse-init-array -emit-obj -D__ORBIS__
ARLAGS      := rc

CFILES      := $(wildcard $(SDIR)/*.c)
CPPFILES    := $(wildcard $(SDIR)/*.cpp)
OBJS        := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES)) $(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(CPPFILES))

# Create the intermediate directory incase it doesn't already exist.
_unused := $(shell mkdir -p $(INTDIR))

# Make rules
ALL: $(ODIR) $(OBJS)
	$(AR) $(ARFLAGS) $(ODIR)/$(LIBNAME) $(ODIR)/*.o

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR):
	@mkdir $@

.PHONY: clean

clean:
	rm -rf $(TARGET) $(ODIR)

install: 
	mkdir -p $(TOOLCHAIN)/include/SDL2
	cp $(INCDIR)/* $(TOOLCHAIN)/include/SDL2
	cp $(ODIR)/$(LIBNAME) $(TOOLCHAIN)/lib
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
CFLAGS      := -cc1 -triple x86_64-pc-freebsd-elf -munwind-tables -fuse-init-array -emit-obj -D__OPENORBIS__ -Wno-logical-op-parentheses -Wno-macro-redefined $(IDIRS)
ARLAGS      := rc

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
CFILES      := $(call rwildcard,$(SDIR),*.c)
CPPFILES    := $(call rwildcard,$(SDIR),*.cpp)
OBJS        := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES)) $(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(CPPFILES))
OBJSOURCES  := $(sort $(dir $(OBJS)))

# Make rules
ALL: $(ODIR) $(OBJS)
	$(AR) $(ARFLAGS) $(ODIR)/$(LIBNAME) $(ODIR)/*.o

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR):
	@mkdir $@
	@mkdir -p $(OBJSOURCES)
	

.PHONY: clean

clean:
	rm -rf $(TARGET) $(ODIR)

install: 
	mkdir -p $(TOOLCHAIN)/include/$(PROJDIR)
	cp $(INCDIR)/* $(TOOLCHAIN)/include/$(PROJDIR)
	cp $(ODIR)/$(LIBNAME) $(TOOLCHAIN)/lib
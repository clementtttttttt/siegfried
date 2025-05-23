# tool macros
CC := x86_64-pc-none-elf-gcc
AR := x86_64-pc-none-elf-ar


INC_PATH:=include

override CFLAGS += -I$(INC_PATH)  -O3 -static

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

SYSROOT := $(HOME)/cross/x86_64-siegfried/sys-root

project_name := hagen

# compile macros
TARGET_NAME := libc.a

TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*))) crt0.S
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: all
# non-phony targets
$(TARGET): $(OBJ)
	$(AR) rcs  -o $@ $(OBJ)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<


$(OBJ_PATH)/crt0.o: $(SRC_PATH)/crt0.S
	$(CC) -c $(CFLAGS) -o $@ $<



# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH)

.PHONY: all
all: $(TARGET)



.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)

install: $(TARGET)
	mkdir -p $(SYSROOT)/usr/include
	cp -Lr include $(SYSROOT)/usr/
	mkdir -p $(SYSROOT)/usr/lib
	install -m 644 $(TARGET) $(SYSROOT)/usr/lib
	install -m 644 $(OBJ_PATH)/crt0.o $(SYSROOT)/usr/lib

# tool macros
CC := x86_64-siegfried-gcc

INC_PATH:=include

CFLAGS := -I$(INC_PATH)


# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

SYSROOT := $(HOME)/cross/x86_64-siegfried

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
	x86_64-siegfried-ar rcs  -o $@ $(OBJ)

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
	cp -r include $(SYSROOT)
	install -m 644 $(TARGET) $(SYSROOT)/lib/
	install -m 644 $(OBJ_PATH)/crt0.o $(SYSROOT)/lib

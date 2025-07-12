define .RECIPEPREFIX
 
endef

MAKEFLAGS=-j 8

BINARY_NAME=bygg
CC=gcc-14
CFLAGS=-Wall -std=c23 -Og -g
LDFLAGS=-Llib
SRC_PATH=
MODULES=bygg
OBJS := $(foreach module,$(MODULES),build/$(module).o)

ifeq ($(DEBUG),DEBUG)
    CFLAGS+= -DDEBUG -g
endif

main: build build/$(BINARY_NAME)

debug: clean main

build:
    mkdir build

build/$(BINARY_NAME): $(OBJS)
    $(CC) -o build/$(BINARY_NAME) $(OBJS) $(LDFLAGS)

build/%.o : $(SRC_PATH)%.c
        $(CC) -MMD -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
    rm -rf build

install: main
    install -Dm755 build/$(BINARY_NAME) /usr/local/bin/


-include $(OBJS:.o=.d)

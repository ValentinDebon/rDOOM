CC=clang
CFLAGS=-g -Wall

LD=$(CC)
LDFLAGS=

BUILD=build
READWAD=readwad

OBJECTS= \
	$(BUILD)/main.o \

.PHONY: all clean

all: $(BUILD)/$(READWAD)

clean:
	rm -f $(BUILD)/*

$(BUILD)/$(READWAD): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<


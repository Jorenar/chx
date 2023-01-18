PROGNAME := chx

BUILD  := build
OBJDIR := $(BUILD)/obj
BINDIR := $(BUILD)/bin
SRCDIR := src

CFLAGS  := -std=c99 -O2
LDFLAGS := -s
LDLIBS  :=

ASAN_FLAGS := -fsanitize=address,undefined,leak,signed-integer-overflow
ASAN_FLAGS := $(ASAN_FLAGS) -fno-omit-frame-pointer

SRCS := $(wildcard $(SRCDIR)/*.c) # temporarly unused
SRCS := $(SRCDIR)/chx.c
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))


$(OBJDIR)/%.o: $(SRCS)
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/%: $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)


.PHONY: build clean debug

build: $(BINDIR)/$(PROGNAME)

clean:
	$(RM) -r build/

debug: CFLAGS += -DDEBUG=1
debug: CFLAGS += -pedantic
debug: CFLAGS += -Wall -Wextra -Wconversion
debug: CFLAGS += -g3 -O0
debug: CFLAGS += $(ASAN_FLAGS)
debug: LDFLAGS :=
debug: LDFLAGS += $(ASAN_FLAGS)
debug: build

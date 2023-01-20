PROGNAME := chx

BUILD    := build
OBJDIR   := $(BUILD)/obj
BINDIR   := $(BUILD)/bin
SRCDIR   := src
DEPSDIR  := $(BUILD)/deps

CFLAGS   := -std=c99 -O2
CPPFLAGS :=

LDFLAGS  := -s
LDLIBS   :=

ASAN_FLAGS := -fsanitize=address,undefined,leak,signed-integer-overflow
ASAN_FLAGS := $(ASAN_FLAGS) -fno-omit-frame-pointer

SRCS := $(wildcard $(SRCDIR)/*.c) # temporarly unused
SRCS := $(SRCDIR)/chx.c
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

.PHONY: build clean debug compile_commands.json

build: $(BINDIR)/$(PROGNAME)

clean:
	$(RM) -r build/

debug: CFLAGS += -DDEBUG=1
debug: CFLAGS += -pedantic
debug: CFLAGS += -Wall -Wextra
debug: CFLAGS += -Wvla -Wconversion -Wformat=2
debug: CFLAGS += -g3 -O0
debug: CFLAGS += $(ASAN_FLAGS)
debug: LDFLAGS :=
debug: LDFLAGS += $(ASAN_FLAGS)
debug: build

compile_commands.json:
	$(MAKE) --always-make --dry-run \
		| grep -wE '$(CC)' \
		| grep -w '\-c' \
		| jq -nR '[inputs|{directory:".", command:., file: match(" [^ ]+$$").string[1:]}]' \
		> compile_commands.json


-include $(patsubst $(OBJDIR)/%.o, $(DEPSDIR)/%.d, $(OBJS))


$(BINDIR)/%: $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(DEPSDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(DEPSDIR)
	$(CC) -M $< -MT $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $<) > $@

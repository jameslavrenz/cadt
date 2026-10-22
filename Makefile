CC ?= cc
CFLAGS ?= -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Werror
CPPFLAGS := -Iinclude -Isrc
BUILD := build

LIB_SRCS := \
	src/common.c \
	src/span.c \
	src/vector.c \
	src/deque.c \
	src/list.c \
	src/forward_list.c \
	src/array.c \
	src/stack.c \
	src/queue.c \
	src/priority_queue.c \
	src/rbtree.c \
	src/set.c \
	src/map.c \
	src/hashtable.c \
	src/unordered_set.c \
	src/unordered_map.c \
	src/string.c \
	src/bitset.c \
	src/optional.c \
	src/algo.c

LIB_OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(LIB_SRCS))

TEST_SRCS := \
	tests/test_main.c \
	tests/track_alloc.c \
	tests/test_vector.c \
	tests/test_deque.c \
	tests/test_list.c \
	tests/test_adapters.c \
	tests/test_assoc.c \
	tests/test_utility.c

.PHONY: all test examples clean install

all: $(BUILD)/libcadt.a

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -O2 $(CPPFLAGS) -c $< -o $@

$(BUILD)/libcadt.a: $(LIB_OBJS)
	ar rcs $@ $(LIB_OBJS)

$(BUILD)/test_cadt: $(LIB_SRCS) $(TEST_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer \
		$(CPPFLAGS) -Itests -o $@ $(LIB_SRCS) $(TEST_SRCS)

$(BUILD)/demo: examples/demo.c $(BUILD)/libcadt.a
	$(CC) $(CFLAGS) -O2 $(CPPFLAGS) -o $@ examples/demo.c -L$(BUILD) -lcadt

$(BUILD)/word_count: examples/word_count.c $(BUILD)/libcadt.a
	$(CC) $(CFLAGS) -O2 $(CPPFLAGS) -o $@ examples/word_count.c -L$(BUILD) -lcadt

examples: $(BUILD)/demo $(BUILD)/word_count

test: $(BUILD)/test_cadt $(BUILD)/demo $(BUILD)/word_count
	$(BUILD)/test_cadt
	$(BUILD)/demo
	printf 'one two two three three three\n' | $(BUILD)/word_count > $(BUILD)/words.out
	cmp tests/word_count.expected $(BUILD)/words.out

PREFIX ?= /usr/local
install: $(BUILD)/libcadt.a
	mkdir -p $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include/cadt
	cp $(BUILD)/libcadt.a $(DESTDIR)$(PREFIX)/lib/libcadt.a
	cp include/cadt.h $(DESTDIR)$(PREFIX)/include/cadt.h
	cp include/cadt/*.h $(DESTDIR)$(PREFIX)/include/cadt/

clean:
	rm -rf $(BUILD)

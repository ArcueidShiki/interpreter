# Build file, won't be submitted.

CC := cc
CFLAGS_DEBUG := -DDEBUG_MODE -Wall -Werror -Wextra -g -std=c11 -fsanitize=address -fsanitize=undefined -O3 -fstrict-aliasing
CFLAGS_RELEASE := -Wall -Werror -Wextra -std=c11 -O3 -fstrict-aliasing
SRC_DIR := src
INC_DIR := include
OUT_DIR := out
TEST_DIR := test
SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS_TEST := $(wildcard $(TEST_DIR)/*.c)
TARGETS_DEBUG := $(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%_debug,$(SRCS))
TARGETS_RELEASE := $(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%_release,$(SRCS))
TARGETS_TEST := $(patsubst $(TEST_DIR)/%.c,$(OUT_DIR)/%,$(SRCS_TEST))

$(OUT_DIR)/%_debug: $(SRC_DIR)/%.c
	if [ ! -d $(OUT_DIR) ]; then mkdir $(OUT_DIR); fi
	$(CC) $(CFLAGS_DEBUG) $< -o $@ -I$(INC_DIR)

$(OUT_DIR)/%_release: $(SRC_DIR)/%.c
	if [ ! -d $(OUT_DIR) ]; then mkdir $(OUT_DIR); fi
	$(CC) $(CFLAGS_RELEASE) $< -o $@ -I$(INC_DIR)

$(OUT_DIR)/%: $(TEST_DIR)/%.c
	if [ ! -d $(OUT_DIR) ]; then mkdir $(OUT_DIR); fi
	$(CC) $(CFLAGS_DEBUG) $< -o $@ -I$(INC_DIR)

debug: $(TARGETS_DEBUG)

release: $(TARGETS_RELEASE)

test: $(TARGETS_TEST)

all: $(TARGETS_DEBUG) $(TARGETS_RELEASE) $(TARGETS_TEST)

clean:
	rm -rf $(OUT_DIR)/*


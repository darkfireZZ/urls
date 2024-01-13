
BUILD_DIR := build

CC := cc
GENERAL_FLAGS := -std=c99 -Wall -Wextra -Wpedantic
RELEASE_FLAGS := -O3
DEV_FLAGS := -Werror -Og -g

CFLAGS := $(GENERAL_FLAGS)

ifdef DEBUG
CFLAGS += $(DEV_FLAGS)
else
CFLAGS += $(RELEASE_FLAGS)
endif

.PHONY: all
all: $(BUILD_DIR)/urls

.PHONY: readme
readme: README.md

README.md: README.md.template $(BUILD_DIR)/urls
	@mkdir -p $(BUILD_DIR)
	@# This is not very elegant, but it works
	$(BUILD_DIR)/urls --help > $(BUILD_DIR)/usage.txt
	sed -e "/^<INSERT_USAGE>$$/{r $(BUILD_DIR)/usage.txt" -e "d}" README.md.template > $@
	rm $(BUILD_DIR)/usage.txt

$(BUILD_DIR)/urls: urls.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ urls.c

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f README.md

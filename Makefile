CXX ?= c++
AR ?= ar
CPPFLAGS := -Isrc -Iinclude
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
BUILD := build
CLI := $(BUILD)/js

.PHONY: all test test-unit test-regression test-sanitize clean
all: $(CLI)

$(BUILD):
	mkdir -p $(BUILD)

$(CLI): src/main.cpp src/Version.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp -o $@

test-unit: $(CLI)
	test "$$($(CLI) --version)" = "JS++ 0.0.0-dev"
	@echo "JS++ foundation smoke passed"

test-regression:
	python3 ../js-regression-suite/run.py --js "$(abspath $(CLI))" --allow-empty

test: test-unit test-regression

test-sanitize:
	mkdir -p $(BUILD)/san
	$(CXX) $(CPPFLAGS) -std=c++17 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -Wall -Wextra -pedantic src/main.cpp -o $(BUILD)/san/js
	ASAN_OPTIONS="$${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/san/js --version

clean:
	rm -rf $(BUILD)

CXX ?= c++
CC ?= cc
AR ?= ar
CPPFLAGS := -Isrc -Iinclude
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
BUILD := build
OBJECT := $(BUILD)/Runtime.o
FRONTEND_OBJECT := $(BUILD)/Frontend.o
CLI := $(BUILD)/js
STATIC := $(BUILD)/libjs.a
SHARED := $(BUILD)/libjs.so

.PHONY: all test test-unit test-regression test-sanitize clean
all: $(CLI) $(STATIC) $(SHARED)
$(BUILD):
	mkdir -p $(BUILD)
$(OBJECT): src/Runtime.cpp src/Runtime.h include/js.h src/Version.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(FRONTEND_OBJECT): src/Frontend.cpp src/Frontend.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(STATIC): $(OBJECT)
	$(AR) rcs $@ $^
$(SHARED): $(OBJECT)
	$(CXX) -shared $^ -o $@
$(CLI): src/main.cpp $(STATIC) $(FRONTEND_OBJECT)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp $(FRONTEND_OBJECT) $(STATIC) -pthread -o $@
$(BUILD)/lifecycle: tests/lifecycle.cpp $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/lifecycle.cpp $(STATIC) -pthread -o $@
$(BUILD)/frontend: tests/frontend.cpp $(FRONTEND_OBJECT)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/frontend.cpp $(FRONTEND_OBJECT) -o $@
test-unit: $(CLI) $(BUILD)/lifecycle $(BUILD)/frontend
	test "$$($(CLI) --version)" = "JS++ 0.0.0-dev"
	./$(BUILD)/lifecycle
	./$(BUILD)/frontend
test-regression: all
	python3 ../js-regression-suite/run.py --js "$(abspath $(CLI))" --include "$(abspath include)" --library "$(abspath $(STATIC))"
test: test-unit test-regression
test-sanitize:
	mkdir -p $(BUILD)/san
	$(CXX) $(CPPFLAGS) -std=c++17 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -Wall -Wextra -pedantic tests/lifecycle.cpp src/Runtime.cpp -pthread -o $(BUILD)/san/lifecycle
	$(CXX) $(CPPFLAGS) -std=c++17 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -Wall -Wextra -pedantic tests/frontend.cpp src/Frontend.cpp -o $(BUILD)/san/frontend
	ASAN_OPTIONS="$${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/san/lifecycle
	ASAN_OPTIONS="$${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/san/frontend
clean:
	rm -rf $(BUILD)

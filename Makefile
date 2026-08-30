CXX ?= c++
CC ?= cc
AR ?= ar
CPPFLAGS := -Isrc -Iinclude
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
CXXFLAGS += -fvisibility=hidden
PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
ABI_MAJOR := 0
BUILD := build
OBJECT := $(BUILD)/Runtime.o
FRONTEND_OBJECT := $(BUILD)/Frontend.o
BYTECODE_OBJECT := $(BUILD)/Bytecode.o
VM_OBJECT := $(BUILD)/VM.o
HEAP_OBJECT := $(BUILD)/Heap.o
CONVERSION_OBJECT := $(BUILD)/Conversion.o
INTRINSICS_OBJECT := $(BUILD)/Intrinsics.o
LIB_OBJECTS := $(OBJECT) $(FRONTEND_OBJECT) $(BYTECODE_OBJECT) $(VM_OBJECT) $(HEAP_OBJECT) $(CONVERSION_OBJECT) $(INTRINSICS_OBJECT)
CLI := $(BUILD)/js
STATIC := $(BUILD)/libjs.a
SHARED_REAL := $(BUILD)/libjs.so.$(ABI_MAJOR)
SHARED := $(BUILD)/libjs.so

.PHONY: all test test-unit test-regression test-preview-contract test-heap test-sanitize install package-test clean
all: $(CLI) $(STATIC) $(SHARED)
$(BUILD):
	mkdir -p $(BUILD)
$(OBJECT): src/Runtime.cpp src/Runtime.h src/Frontend.h src/Bytecode.h src/VM.h include/js.h src/Version.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(FRONTEND_OBJECT): src/Frontend.cpp src/Frontend.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(BYTECODE_OBJECT): src/Bytecode.cpp src/Bytecode.h src/Frontend.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(VM_OBJECT): src/VM.cpp src/VM.h src/Bytecode.h src/Runtime.h src/Conversion.h src/Error.h src/Property.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(HEAP_OBJECT): src/Heap.cpp src/VM.h src/Runtime.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(CONVERSION_OBJECT): src/Conversion.cpp src/Conversion.h src/Runtime.h src/VM.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(INTRINSICS_OBJECT): src/Intrinsics.cpp src/Intrinsics.h src/Property.h src/Error.h src/VM.h src/Runtime.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(STATIC): $(LIB_OBJECTS)
	$(AR) rcs $@ $^
$(SHARED_REAL): $(LIB_OBJECTS) packaging/js.map
	$(CXX) -shared -Wl,-soname,libjs.so.$(ABI_MAJOR) -Wl,--version-script=packaging/js.map $(LIB_OBJECTS) -o $@
$(SHARED): $(SHARED_REAL)
	ln -sf libjs.so.$(ABI_MAJOR) $@
$(CLI): src/main.cpp $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp $(STATIC) -pthread -o $@
$(BUILD)/lifecycle: tests/lifecycle.cpp $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/lifecycle.cpp $(STATIC) -pthread -o $@
$(BUILD)/frontend: tests/frontend.cpp $(FRONTEND_OBJECT)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/frontend.cpp $(FRONTEND_OBJECT) -o $@
$(BUILD)/vm: tests/vm.cpp $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/vm.cpp $(STATIC) -pthread -o $@
$(BUILD)/heap: tests/heap.cpp $(STATIC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/heap.cpp $(STATIC) -pthread -o $@
test-heap: $(BUILD)/heap
	./$(BUILD)/heap
test-unit: $(CLI) $(BUILD)/lifecycle $(BUILD)/frontend $(BUILD)/vm test-heap
	test "$$($(CLI) --version)" = "JS++ 0.0.0-dev"
	./$(BUILD)/lifecycle
	./$(BUILD)/frontend
	./$(BUILD)/vm
test-regression: all
	python3 ../js-regression-suite/run.py --js "$(abspath $(CLI))" --include "$(abspath include)" --library "$(abspath $(STATIC))"
test-preview-contract:
	python3 ../js-regression-suite/validate_preview_contract.py
test: test-preview-contract test-unit test-regression
install: all
	install -d "$(DESTDIR)$(INCLUDEDIR)" "$(DESTDIR)$(LIBDIR)/pkgconfig" "$(DESTDIR)$(LIBDIR)/cmake/JSpp" "$(DESTDIR)$(BINDIR)"
	install -m 0644 include/js.h "$(DESTDIR)$(INCLUDEDIR)/js.h"
	install -m 0644 $(STATIC) "$(DESTDIR)$(LIBDIR)/libjs.a"
	install -m 0755 $(SHARED_REAL) "$(DESTDIR)$(LIBDIR)/libjs.so.$(ABI_MAJOR)"
	ln -sf libjs.so.$(ABI_MAJOR) "$(DESTDIR)$(LIBDIR)/libjs.so"
	install -m 0755 $(CLI) "$(DESTDIR)$(BINDIR)/js"
	sed 's|@PREFIX@|$(PREFIX)|g' packaging/jspp.pc.in > "$(DESTDIR)$(LIBDIR)/pkgconfig/jspp.pc"
	sed 's|@PREFIX@|$(PREFIX)|g' packaging/JSppConfig.cmake.in > "$(DESTDIR)$(LIBDIR)/cmake/JSpp/JSppConfig.cmake"
package-test: all
	bash tests/package.sh
test-sanitize:
	mkdir -p $(BUILD)/san
	$(CXX) $(CPPFLAGS) -std=c++17 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -Wall -Wextra -pedantic tests/lifecycle.cpp src/Runtime.cpp src/Frontend.cpp src/Bytecode.cpp src/VM.cpp src/Heap.cpp src/Conversion.cpp src/Intrinsics.cpp -pthread -o $(BUILD)/san/lifecycle
	$(CXX) $(CPPFLAGS) -std=c++17 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -Wall -Wextra -pedantic tests/frontend.cpp src/Frontend.cpp -o $(BUILD)/san/frontend
	$(CXX) $(CPPFLAGS) -std=c++17 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -Wall -Wextra -pedantic tests/vm.cpp src/Runtime.cpp src/Frontend.cpp src/Bytecode.cpp src/VM.cpp src/Heap.cpp src/Conversion.cpp src/Intrinsics.cpp -pthread -o $(BUILD)/san/vm
	ASAN_OPTIONS="$${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/san/lifecycle
	ASAN_OPTIONS="$${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/san/frontend
	ASAN_OPTIONS="$${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/san/vm
clean:
	rm -rf $(BUILD)


CXX ?= clang++
CXXFLAGS ?= -Wall -Wextra -std=c++17 -Wpedantic -Wconversion -fsanitize=address -g
LDFLAGS ?=

# SOURCE_FILES := $(shell find src -name '*.cc' -type f)
# OBJECTS      := $(addsuffix .o,$(basename $(SOURCE_FILES)))


# all: $(OBJECTS)

# %.o: %.cc
# 	mkdir -p $(dir build/$@)
# 	$(CXX) $(CXXFLAGS) -c $^ -o build/$@

# clean:
# 	rm -rf build/

main: main.cc
	$(CXX) $(CXXFLAGS) main.cc -o main

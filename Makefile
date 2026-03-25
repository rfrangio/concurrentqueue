CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -pthread
TARGET := cqueue
SOURCES := main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) concurrent_queue.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

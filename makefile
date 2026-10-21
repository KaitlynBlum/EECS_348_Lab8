
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

TARGET = calculator
SOURCE = main.cpp
TESTS = tests.txt
OUTPUT = output.txt

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)

run: $(TARGET) $(TESTS)
	./$(TARGET) $(TESTS) > $(OUTPUT)
	@echo "Test run complete. Results saved to $(OUTPUT)."

clean:
	rm -f $(TARGET)

.PHONY: all clean

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -O3
WARNINGS = -g -Wall

TARGET = Legalizer

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(WARNINGS) main.cpp -o $(TARGET)

clean:
	rm -rf $(TARGET) *.txt *.png *.blocks

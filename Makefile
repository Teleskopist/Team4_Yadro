CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET   = media_finder
SRC      = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	@mkdir -p ~/.media_files
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean

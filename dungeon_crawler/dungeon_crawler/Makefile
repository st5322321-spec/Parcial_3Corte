CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude
SRC      := src/main.cpp src/map.cpp src/entities.cpp src/renderer.cpp \
            src/input.cpp src/highscore.cpp
TARGET   := dungeon_crawler

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) scores.txt

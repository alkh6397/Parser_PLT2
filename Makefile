CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = parser_prog
SRCS = main.cpp Lexer.cpp Parser.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean

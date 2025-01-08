CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -I ../external/utest.h -I ../ -g
HXX = ../BitBuffer.hpp
OBJ = ../BitBuffer.o
INC_SRC = ../BitBuffer.cpp

TARGET = run.out

test: $(TARGET) 
	./$(TARGET)

$(TARGET): main.cpp $(HXX) $(OBJ)
	$(CXX) $< -o $@ $(OBJ) $(CXXFLAGS)

$(OBJ): $(INC_SRC)
	make -C ../

clean:
	$(RM) $(TARGET)

.PHONY: clean test

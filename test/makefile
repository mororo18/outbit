CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -I ../external/utest.h -I ../
HXX = ../Encoder.hpp
OBJ = ../Encoder.o
INC_SRC = ../Encoder.cpp

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

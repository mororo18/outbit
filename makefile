CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -g
OBJ = BitBuffer.o
TEST_DIR = test/

$(OBJ): BitBuffer.cpp BitBuffer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJ)

all: $(OBJ) test

test:
	make -C $(TEST_DIR)

clean:
	$(RM) $(OBJ) 
	make -C $(TEST_DIR) clean

.PHONY: clean test


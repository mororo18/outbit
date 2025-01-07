CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -g
OBJ = Encoder.o
TEST_DIR = test/

$(OBJ): Encoder.cpp Encoder.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJ)

all: $(OBJ) test

test:
	make -C $(TEST_DIR)

clean:
	$(RM) $(OBJ) 
	make -C $(TEST_DIR) clean

.PHONY: clean test


CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b
OBJ = Encoder.o
TEST_DIR = test/

$(OBJ): Encoder.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJ)

all: $(OBJ) test

test:
	make -C $(TEST_DIR)

clean:
	$(RM) $(OBJ) 
	make -C $(TEST_DIR) clean

.PHONY: clean test


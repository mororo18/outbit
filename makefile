CXXFLAGS = -Wall -Wextra -pedantic
OBJ = Encoder.o
SRC = Encoder.cpp
TEST_DIR = test/

$(OBJ): $(SRC) Encoder.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $(OBJ)

test:
	make -C $(TEST_DIR)

clean:
	$(RM) $(OBJ) 
	make -C $(TEST_DIR) clean

.PHONY: clean test


CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -g
OBJ = BitBuffer.o
TEST_DIR = test/

$(OBJ): BitBuffer.cpp BitBuffer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJ)

all: $(OBJ) test

lib: $(OBJ)
	@if [ -z "$(M)" ]; then \
		echo "Variable 'M' not defined." \
		"This variable should be set with the path in which the BitBuffer.o should be outputed."; \
		exit 1; \
	fi
	mv $(OBJ) $(M)

test:
	make -C $(TEST_DIR)

clean-lib: clean
	$(RM) $(M)/$(OBJ)

clean:
	$(RM) $(OBJ) 
	make -C $(TEST_DIR) clean

.PHONY: clean test


CXXFLAGS = -Wall -Wextra -pedantic -I ../external/utest.h -I ../
TARGET = run.out
OBJ = ../Encoder.o

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CXX) main.cpp $< -o $@ $(CXXFLAGS)

$(OBJ):
	make -C ../

clean:
	$(RM) $(TARGET)

.PHONY: clean test

CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -I ../external/utest.h -I ../
HXX = ../Encoder.hpp
TARGET = run.out

test: $(TARGET) 
	./$(TARGET)

$(TARGET): main.cpp  $(HXX)
	$(CXX) $< -o $@ $(CXXFLAGS)

clean:
	$(RM) $(TARGET)

.PHONY: clean test

# Variáveis
CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Iexternal/utest.h
OBJ = Encoder.o
SRC = Encoder.cpp
TARGET = test_app
TEST_SRCS = test/main.cpp Encoder.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Regra padrão
all: $(OBJ)

# Regra para compilar o arquivo objeto
$(OBJ): $(SRC) Encoder.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $(OBJ)

# Regra para compilar e rodar os testes
test: $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(TEST_OBJS)
	./$(TARGET)

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJ) $(TEST_OBJS) $(TARGET)

# Regra padrão para evitar erros ao digitar "make clean" antes do "all"
.PHONY: all clean test


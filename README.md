# outbit

A simple C++20 library for handling arbitrary bit lengths, ideal for custom encoding and compression.

## setup
```
git clone --recurse-submodules https://github.com/mororo18/outbit.git
```
or
```
git clone https://github.com/mororo18/outbit.git
cd outbit
git submodule update --init --recursive
```

## build

```
make
```

## build and clean as dependency

```
# build BitBuffer.o and move to specified path
make -C path/to/outbit M=path/to/output/object/file lib

# clean BitBuffer.o from specified path
make -C path/to/outbit M=path/to/output/object/file clean-lib
```

## build and run tests

```
make test
```

## usage examples

Read arbitrary bit lengths in sequence:
```cpp
// Create an empty buffer
auto bitbuffer = outbit::BitBuffer();
// Load a file into the internal buffer
bitbuffer.read_from_file("custom.file");

// Read the first bytes as a integer
auto var = bitbuffer.read_as<int64_t>();

// Read the next 11 bits as an int
auto first = bitbuffer.read_bits_as<int>(11);
// Read the next 5 bits as a char
auto second = bitbuffer.read_bits_as<char>(5);
```

Write arbitrary bit lengths in sequence:

```cpp
// Create an empty buffer
auto bitbuffer = outbit::BitBuffer();

int64_t var = -42;

// Write integer to the buffer as raw bytes
bitbuffer.write(var);

uint32_t first_value = 1324;
char second_value = '@';

// Write the first 11 bits of 'first_value' in the buffer
bitbuffer.write_bits(first_value, 11);
// Write the first 5 bits of 'second_value' in the buffer
bitbuffer.write_bits(second_value, 5);

// Save the buffer to a file
bitbuffer.write_as_file("custom.file");
```

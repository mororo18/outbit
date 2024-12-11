#ifndef __ENCODER_HPP_
#define __ENCODER_HPP_

#include <vector>
#include <cmath>
#include <bitset>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <utest.h>

typedef uint8_t byte_t;
typedef uint64_t index_t;

class Encoder {
public:
    Encoder();

    void store (index_t value, uint8_t nbits);

    index_t load (uint8_t nbits);
    index_t loadAndOffset (uint8_t nbits);
    void    resetOffset();

    void write (std::string name);
    void write ();

    void read  (std::string filename);

    const std::vector<byte_t>& output_buffer();

    void printInput ();
    void printOutput ();
private:
    std::vector<byte_t> input;
    size_t              in_index_offset;    // indice que aponta para byte que contem o proximo numero
    byte_t              in_bit_offset;      // offset em bits do proximo numero

    size_t  in_index_offset_ref;
    byte_t  in_bit_offset_ref;

    std::vector<byte_t> output;
    byte_t buff;
    byte_t buff_bits;

    std::vector<byte_t> buff_vec;
    std::string output_name;
};

#endif

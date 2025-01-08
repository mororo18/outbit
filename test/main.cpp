#include "utest.h"
#include "BitBuffer.hpp"
#include <limits>

using namespace outbit;

UTEST(BitBuffer, roundtrip) {
    // rm warnings
    ASSERT_TRUE(true);
}

UTEST(BitBuffer, from_bits_to_bytes_lentgh) {
    ASSERT_EQ(size_t(0), BitBuffer::from_bits_to_bytes_length(0));
    ASSERT_EQ(size_t(1), BitBuffer::from_bits_to_bytes_length(1));
    ASSERT_EQ(size_t(1), BitBuffer::from_bits_to_bytes_length(7));
    ASSERT_EQ(size_t(1), BitBuffer::from_bits_to_bytes_length(8));
    ASSERT_EQ(size_t(2), BitBuffer::from_bits_to_bytes_length(9));
    ASSERT_EQ(size_t(9), BitBuffer::from_bits_to_bytes_length(65));
}

UTEST(BitBuffer, read_bits_as) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    auto data = std::vector<u8> {64, 1, 128, 1};
    auto bitbuff = BitBuffer();
    bitbuff.read_from_vector(data);

    auto first_struct = bitbuff.read_bits_as<Bytes>(8);
    ASSERT_EQ(first_struct.a, 64);
    ASSERT_EQ(first_struct.b, 0);
    ASSERT_EQ(first_struct.c, 0);

    auto second_struct = bitbuff.read_bits_as<Bytes>(1);
    ASSERT_EQ(second_struct.a, 1);
    ASSERT_EQ(second_struct.b, 0);
    ASSERT_EQ(second_struct.c, 0);

    auto third_struct = bitbuff.read_bits_as<Bytes>(1);
    ASSERT_EQ(third_struct.a, 0);

    auto fourth_struct = bitbuff.read_bits_as<Bytes>(6);
    ASSERT_EQ(fourth_struct.a, 0);

    auto fifth_ret = bitbuff.read_bits_as<int>(9);
    ASSERT_EQ(fifth_ret, 0b110000000);
}

UTEST(BitBuffer, write) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    Bytes bytes = { 255, 255, 255 };

    auto bitbuff = BitBuffer();
    bitbuff.write_bits(bytes, 10);
    ASSERT_EQ(bitbuff.tail_byte().value(), 3);

    auto before_write = bitbuff.buffer().size();
    bitbuff.write(bytes);
    auto after_write = bitbuff.buffer().size();
    ASSERT_EQ(bitbuff.tail_byte().value(), 3);
    ASSERT_EQ(before_write + sizeof(bytes), after_write);
}

UTEST(BitBuffer, write_bits) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    Bytes bytes = { 255, 255, 255 };

    auto bitbuff = BitBuffer();
    bitbuff.write_bits(bytes, 8);
    ASSERT_EQ(bitbuff.tail_byte().value(), 255);

    bitbuff.write_bits(bytes, 9);
    ASSERT_EQ(bitbuff.tail_byte().value(), 1);

    bitbuff.write_bits(bytes, 9);
    ASSERT_EQ(bitbuff.tail_byte().value(), 3);

    bitbuff.write_bits(bytes, 8);
    ASSERT_EQ(bitbuff.tail_byte().value(), 3);

    bitbuff.write_bits(0b111111, 6);
    ASSERT_EQ(bitbuff.tail_byte().value(), 255);


    /*
    std::println("buffer size {}", bitbuff.buffer().size());
    std::println("buffer as bits:");
    for (auto byte: bitbuff.buffer()) {
        std::println("{}", std::bitset<8>{byte}.to_string());
    }
    */
}

UTEST(BitBuffer, const_buffer) {
    auto bitbuff = BitBuffer();
    auto size_before = bitbuff.buffer().size();
    auto buffer = bitbuff.buffer();
    buffer.push_back(1);
    auto size_after = bitbuff.buffer().size();

    ASSERT_EQ(size_before, size_after);
}

UTEST(BitBuffer, serialize_bytes) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    Bytes bytes = { 10, 1, 127 };

    auto serialized_bytes = BitBuffer::serialize(bytes);

    ASSERT_EQ(serialized_bytes.at(0), 10);
    ASSERT_EQ(serialized_bytes.at(1), 1);
    ASSERT_EQ(serialized_bytes.at(2), 127);
}

UTEST(BitBuffer, serialize_integers) {

    typedef struct integers {
        int32_t a;
        int32_t b;
        uint32_t c;
    } Integers;

    Integers integers = {
        10,
        std::numeric_limits<int32_t>::max(),
        std::numeric_limits<uint32_t>::max(),
    };

    auto serialized_integers = BitBuffer::serialize(integers);

    ASSERT_EQ(serialized_integers.size(), sizeof(Integers));

    ASSERT_EQ(serialized_integers.at(0), 10);
    ASSERT_EQ(serialized_integers.at(1), 0);
    ASSERT_EQ(serialized_integers.at(2), 0);
    ASSERT_EQ(serialized_integers.at(3), 0);

    ASSERT_EQ(serialized_integers.at(4), 255);
    ASSERT_EQ(serialized_integers.at(5), 255);
    ASSERT_EQ(serialized_integers.at(6), 255);
    ASSERT_EQ(serialized_integers.at(7), 127);

    ASSERT_EQ(serialized_integers.at(8), 255);
    ASSERT_EQ(serialized_integers.at(9), 255);
    ASSERT_EQ(serialized_integers.at(10), 255);
    ASSERT_EQ(serialized_integers.at(11), 255);
}

UTEST_MAIN()

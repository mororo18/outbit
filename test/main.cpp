#include "utest.h"
#include "Encoder.hpp"
#include <limits>

using namespace outbit;

UTEST(Encoder, roundtrip) {
    auto enc = Encoder();
    enc.write_as_file("eita");

    // rm warnings
    ASSERT_TRUE(true);
}

UTEST(Encoder, from_bits_to_bytes_lentgh) {
    ASSERT_EQ(size_t(0), Encoder::from_bits_to_bytes_length(0));
    ASSERT_EQ(size_t(1), Encoder::from_bits_to_bytes_length(1));
    ASSERT_EQ(size_t(1), Encoder::from_bits_to_bytes_length(7));
    ASSERT_EQ(size_t(1), Encoder::from_bits_to_bytes_length(8));
    ASSERT_EQ(size_t(2), Encoder::from_bits_to_bytes_length(9));
    ASSERT_EQ(size_t(9), Encoder::from_bits_to_bytes_length(65));
}

UTEST(Encoder, read_bits_as) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    auto data = std::vector<u8> {64, 1, 128, 1};
    auto enc = Encoder();
    enc.read_from_vector(data);

    auto first_struct = enc.read_bits_as<Bytes>(8);
    ASSERT_EQ(first_struct.a, 64);
    ASSERT_EQ(first_struct.b, 0);
    ASSERT_EQ(first_struct.c, 0);

    auto second_struct = enc.read_bits_as<Bytes>(1);
    ASSERT_EQ(second_struct.a, 1);
    ASSERT_EQ(second_struct.b, 0);
    ASSERT_EQ(second_struct.c, 0);

    auto third_struct = enc.read_bits_as<Bytes>(1);
    ASSERT_EQ(third_struct.a, 0);

    auto fourth_struct = enc.read_bits_as<Bytes>(6);
    ASSERT_EQ(fourth_struct.a, 0);

    auto fifth_ret = enc.read_bits_as<int>(9);
    ASSERT_EQ(fifth_ret, 0b110000000);
}

UTEST(Encoder, write) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    Bytes bytes = { 255, 255, 255 };

    auto enc = Encoder();
    enc.write_bits(bytes, 10);
    ASSERT_EQ(enc.tail_byte().value(), 3);

    auto before_write = enc.buffer().size();
    enc.write(bytes);
    auto after_write = enc.buffer().size();
    ASSERT_EQ(enc.tail_byte().value(), 3);
    ASSERT_EQ(before_write + sizeof(bytes), after_write);
}

UTEST(Encoder, write_bits) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    Bytes bytes = { 255, 255, 255 };

    auto enc = Encoder();
    enc.write_bits(bytes, 8);
    ASSERT_EQ(enc.tail_byte().value(), 255);

    enc.write_bits(bytes, 9);
    ASSERT_EQ(enc.tail_byte().value(), 1);

    enc.write_bits(bytes, 9);
    ASSERT_EQ(enc.tail_byte().value(), 3);

    enc.write_bits(bytes, 8);
    ASSERT_EQ(enc.tail_byte().value(), 3);

    enc.write_bits(0b111111, 6);
    ASSERT_EQ(enc.tail_byte().value(), 255);


    /*
    std::println("buffer size {}", enc.buffer().size());
    std::println("buffer as bits:");
    for (auto byte: enc.buffer()) {
        std::println("{}", std::bitset<8>{byte}.to_string());
    }
    */
}

UTEST(Encoder, const_buffer) {
    auto enc = Encoder();
    auto size_before = enc.buffer().size();
    auto buffer = enc.buffer();
    buffer.push_back(1);
    auto size_after = enc.buffer().size();

    ASSERT_EQ(size_before, size_after);
}

UTEST(Encoder, serialize_bytes) {
    typedef struct bytes {
        u8 a;
        u8 b;
        u8 c;
    } Bytes;

    Bytes bytes = { 10, 1, 127 };

    auto serialized_bytes = Encoder::serialize(bytes);

    ASSERT_EQ(serialized_bytes.at(0), 10);
    ASSERT_EQ(serialized_bytes.at(1), 1);
    ASSERT_EQ(serialized_bytes.at(2), 127);
}

UTEST(Encoder, serialize_integers) {

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

    auto serialized_integers = Encoder::serialize(integers);

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

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <utest.h>
#include <BitBuffer.hpp>

using namespace outbit;

UTEST(BitBuffer, roundtrip) {
    std::ifstream file("bit_value_pairs.txt");
    
    ASSERT_TRUE(file.is_open());

    size_t bit_count, value;
    struct bit_value_pair {
        size_t bit_count, value;
    };

    std::vector<struct bit_value_pair> bit_value_pairs;
    while (file >> bit_count >> value) {
        bit_value_pairs.push_back({bit_count, value});
    }

    file.close();

    auto bit_writer = BitBuffer();
    for (auto [bit_count, value] : bit_value_pairs) {
        bit_writer.write_bits(value, bit_count);
    }

    auto writer_output = std::vector<u8>(bit_writer.buffer());

    auto bit_reader = BitBuffer();
    auto new_bit_writer = BitBuffer();
    bit_reader.read_from_vector(writer_output);
    for (auto [bit_count, _] : bit_value_pairs) {
        auto value = bit_reader.read_bits_as<size_t>(bit_count);
        new_bit_writer.write_bits(value, bit_count);
    }

    auto bit_writer_buffer = bit_writer.buffer();
    auto new_bit_writer_buffer = new_bit_writer.buffer();

    ASSERT_EQ(bit_writer_buffer.size(), new_bit_writer_buffer.size());

    auto byte = bit_writer_buffer.begin();
    auto byte_end = bit_writer_buffer.end();

    auto round_byte = new_bit_writer_buffer.begin();
    auto round_byte_end = new_bit_writer_buffer.end();

    while (byte != byte_end && round_byte != round_byte_end) {
        ASSERT_EQ(*byte++, *round_byte++);
    }
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

    /*
    typedef struct integers {
        int32_t a;
        uint64_t b;
        int16_t c;
    } Integers;

    Integers myints = {
        .a = -53909,
        .b = 2326172,
        .c = 41,
    };

    bitbuff = BitBuffer();
    bitbuff.write(myints);
    auto readed_ints = bitbuff.read_as<Integers>();

    ASSERT_EQ(readed_ints.a, myints.a);
    ASSERT_EQ(readed_ints.b, myints.b);
    ASSERT_EQ(readed_ints.c, myints.c);
    */
}

// TODO: Add more operations
UTEST(BitBuffer, alternating_write_and_read_ops) {
    auto bitbuff = BitBuffer();
    const int a = 42;
    bitbuff.write(a);

    ASSERT_EQ(a, bitbuff.read_as<int>());

    typedef struct integers {
        int32_t a;
        uint64_t b;
        int16_t c;
    } Integers;

    const Integers myints = {
        .a = -53909,
        .b = 2326172,
        .c = 41,
    };

    bitbuff.write(myints);

    bitbuff.write(a);
    ASSERT_EQ(a, bitbuff.read_as<int>());

    auto read_ints = bitbuff.read_as<Integers>();

    ASSERT_EQ(read_ints.a, myints.a);
    ASSERT_EQ(read_ints.b, myints.b);
    ASSERT_EQ(read_ints.c, myints.c);
}

// TODO: Add more structs
UTEST(BitBuffer, write_and_read_of_big_structs) {
    typedef struct integers {
        int8_t a;
        int16_t aa;
        int32_t aaa;
        int64_t aaaa;

        uint8_t b;
        uint16_t bb;
        uint32_t bbb;
        uint64_t bbbb;

    } AllIntegers;

    const AllIntegers myints = {
        .a = -50,
        .aa = -2372,
        .aaa = 41,
        .aaaa = -4342341987,

        .b = 50,
        .bb = 2372,
        .bbb = 41,
        .bbbb = 4342341987,
    };

    auto bitbuff = BitBuffer();
    bitbuff.write(myints);
    auto read = bitbuff.read_as<AllIntegers>();

    ASSERT_EQ(myints.a, read.a);
    ASSERT_EQ(myints.aa, read.aa);
    ASSERT_EQ(myints.aaa, read.aaa);
    ASSERT_EQ(myints.aaaa, read.aaaa);

    ASSERT_EQ(myints.b, read.b);
    ASSERT_EQ(myints.bb, read.bb);
    ASSERT_EQ(myints.bbb, read.bbb);
    ASSERT_EQ(myints.bbbb, read.bbbb);
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

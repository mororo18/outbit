#include "utest.h"
#include "Encoder.hpp"
#include <limits>

using namespace outbit;

UTEST(Test, Main) {
    ASSERT_TRUE(true);
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

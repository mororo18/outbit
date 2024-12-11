#include <utest.h>
#include "../Encoder.hpp"

UTEST(Test, Main) {
    ASSERT_TRUE(true);
}

UTEST(Encoder, ouput_buffer) {
    Encoder enc;

    enc.store(10, 4);
    enc.store(1, 5);

    auto out_buff = enc.output_buffer();

    ASSERT_EQ(out_buff.size(), 2);
}

UTEST_MAIN()

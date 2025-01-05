#ifndef __ENCODER_HPP_
#define __ENCODER_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include <bitset>
#include <ranges>

namespace outbit {
    typedef uint8_t u8;
    typedef int8_t s8;
    class Encoder {
        public:
            Encoder();

            template<typename T> static
            std::vector<u8> serialize(const T &item);
            template<typename T>
            void push(const T &item);
            template<typename T>
            void push_bits(const T &item, std::size_t n_bits);
        private:
            std::vector<u8> m_buffer{};
            s8 m_tail_byte_used_bits = 0;
    };

    template<typename T>
    std::vector<u8> Encoder::serialize(const T &item) {
        auto addr = const_cast<u8*>(reinterpret_cast<const u8*>(&item));
        auto item_bytes = sizeof(T);

        auto serialized = std::vector<u8>();
        serialized.assign(addr, addr + item_bytes);
        return serialized;
    }

    template<typename T>
    void Encoder::push(const T &item) {
        // FIXME: consider the tail byte used bits;

        std::vector<u8> serialized = Encoder::serialize(item);
        m_buffer.insert(m_buffer.end(), serialized.begin(), serialized.end());
    }

    template<typename T>
    void Encoder::push_bits(const T &item, std::size_t n_bits) {
        assert(n_bits <= sizeof(T));
        // We add 8 bits to do some bitwise operations
        std::bitset<sizeof(T) + 8> item_bits;

        // fill bitset
        std::vector<u8> serialized = Encoder::serialize(item);
        for (auto [byte_index, byte] : std::views::enumerate(serialized)) {
            std::bitset<8> byte_bits{byte};
            for (int bit_index = 0; bit_index < 8; bit_index++) {
                // The 'bitset::test' function performs a bounds check
                item_bits.test(byte_index * 8 + bit_index);
                item_bits[byte_index * 8 + bit_index] = byte_bits.test(bit_index);
            }
        }

        if (m_tail_byte_used_bits > 0) {
            assert(m_tail_byte_used_bits <= 8);
            
            u8 tail_byte = m_buffer.back();
            std::bitset<8> tail_bits {tail_byte};

            item_bits >>= m_tail_byte_used_bits;
            item_bits &= tail_bits;

            m_buffer.pop_back();
        }

        std::size_t output_bits = m_tail_byte_used_bits + n_bits;


        // TODO: We need to zero the remaning bits at the end.
    }
}

#endif

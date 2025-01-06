#ifndef __ENCODER_HPP_
#define __ENCODER_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include <bitset>
#include <ranges>
#include <print>
#include <optional>

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
            inline std::optional<u8> tail_byte() const;
            inline const std::vector<u8>& buffer();
        private:
            std::vector<u8> m_buffer{};
            s8 m_used_length_of_tail_byte = 0;
    };

    const std::vector<u8>& Encoder::buffer() {
        return m_buffer;
    }

    std::optional<u8> Encoder::tail_byte() const {
        if (!m_buffer.empty()) {
            return std::optional<u8>{m_buffer.back()};
        } else {
            return std::nullopt;
        }
    }

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
        this->push_bits(item, sizeof(T) * 8);
    }

    template<typename T>
    void Encoder::push_bits(const T &item, std::size_t n_bits) {
        const std::size_t item_bits_lenght = sizeof(T) * 8;
        assert(n_bits <= item_bits_lenght);
        
        // We add 8 bits to have the option of do a right bit shift
        // and place the tail byte used bits at the beggining of the
        // bitset.
        auto item_bits = std::bitset<item_bits_lenght + 8>{};

        // fill bitset
        auto serialized = Encoder::serialize(item);
        for (auto [byte_index, byte] : std::views::enumerate(serialized)) {
             auto byte_bits = std::bitset<8>{byte};
            for (int bit_index = 0; bit_index < 8; bit_index++) {
                // The 'bitset::test' function performs a bounds check
                item_bits.test(byte_index * 8 + bit_index);
                item_bits[byte_index * 8 + bit_index] = byte_bits.test(bit_index);
            }
        }
        assert(m_used_length_of_tail_byte <= 8);

        std::size_t output_valid_bits_lenght;
        if (m_used_length_of_tail_byte > 0 && m_used_length_of_tail_byte < 8) {
            
            u8 tail_byte = this->tail_byte().value();
            auto tail_bits = std::bitset<item_bits_lenght + 8>{tail_byte};

            item_bits <<= m_used_length_of_tail_byte;
            item_bits |= tail_bits;

            m_buffer.pop_back();

            output_valid_bits_lenght = m_used_length_of_tail_byte + n_bits;
        } else {
            output_valid_bits_lenght = n_bits;
        }

        auto all_one_bits = std::bitset<item_bits_lenght + 8>{0}.flip();
        auto output_valid_bits  = (all_one_bits << output_valid_bits_lenght).flip();
        auto output_bits = item_bits & output_valid_bits;
        auto output = Encoder::serialize(output_bits.to_ullong());

        assert(
                (output_valid_bits_lenght + 8 - (output_valid_bits_lenght + 8) % 8) % 8 == 0
              );

        std::size_t output_valid_bytes_lenght;
        if (output_valid_bits_lenght % 8) {
            output_valid_bytes_lenght =
                (output_valid_bits_lenght + 8 - (output_valid_bits_lenght + 8) % 8) / 8;
            m_used_length_of_tail_byte = output_valid_bits_lenght % 8;
        } else {
            output_valid_bytes_lenght = output_valid_bits_lenght / 8;
            m_used_length_of_tail_byte = 8;
        }

        assert(output_valid_bytes_lenght <= output.size());

        /*
        std::println("pushed {} bits of item with {} bits", n_bits, item_bits_lenght);
        std::println("output valid bits  {}", output_valid_bits_lenght);
        std::println("output valid bytes {}", output_valid_bytes_lenght);
        std::println("output bits        {}", output_bits.to_string());
        */

        m_buffer.insert(m_buffer.end(),
                output.begin(), output.begin() + output_valid_bytes_lenght);

    }
}

#endif

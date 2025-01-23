#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>
#include <bitset>
#include <ranges>
#include <print>
#include <optional>
#include <filesystem>
#include <source_location>
#include <span>
#include <algorithm>
#include <cstdio>

namespace outbit {
    namespace fs = std::filesystem;
    const int BYTE_BITS = 8;

    using u8 = uint8_t;
    using s8 = int8_t;

    class IndexedSpan {
        public:
            IndexedSpan() = default;
            IndexedSpan(std::size_t offset, std::size_t count);
            template<typename T>
            std::optional<std::span<T>> get_span_from_vector(std::vector<T>& vec);
            bool empty() { return m_count == 0; }
            std::size_t offset() { return m_offset; }
            std::optional<IndexedSpan> subspan(std::size_t offset);
            std::optional<IndexedSpan> subspan(std::size_t offset, std::size_t count);
        private:
            std::size_t m_offset;
            std::size_t m_count;
    };

    template<typename T>
    std::optional<std::span<T>> IndexedSpan::get_span_from_vector(std::vector<T>& vec) {
        if (m_offset + m_count > vec.size()) {
            return std::nullopt;
        }

        return std::span<T>(vec.data(), vec.size())
            .subspan(m_offset, m_count);
    }

    class BitBuffer {
        public:
            BitBuffer() = default;
            void read_from_file(const fs::path& filepath,
                    std::source_location = std::source_location::current());

            template<typename T>
            void read_from_span(std::span<T> slice);
            template<typename T>
            void read_from_vector(std::vector<T>& slice);

            constexpr
            static std::size_t constexpr_from_bits_to_bytes_length(std::size_t nbits);
            static std::size_t from_bits_to_bytes_length(std::size_t nbits);

            void write_as_file(const fs::path&,
                    std::source_location = std::source_location::current());

            template<typename T, std::size_t N = sizeof(T)> static
            std::array<u8, N> serialize(const T& item);

            template<std::size_t N,
                std::size_t B = BitBuffer::constexpr_from_bits_to_bytes_length(N)>
            static std::array<u8, B> serialize_bitset(const std::bitset<N>& bits);

            template<typename T>
            T read_as();
            template<typename T>
            T read_bits_as(std::size_t n_bits);

            template<typename T>
            void write(const T& item);

            template<typename T>
            void write_bits(const T& item, std::size_t n_bits);
            inline std::optional<u8> tail_byte();
            inline const std::vector<u8>& buffer();

        private:
            std::vector<u8> m_buffer;
            IndexedSpan m_buffer_slice;
            std::size_t m_used_length_of_tail_byte = 0;
            std::size_t m_unread_bits_of_head_byte = 0;
    };

    const std::vector<u8>& BitBuffer::buffer() {
        return m_buffer;
    }

    std::optional<u8> BitBuffer::tail_byte() {
        if (!m_buffer.empty()) {
            return std::optional<u8>{m_buffer.back()};
        } 

        return std::nullopt;
    }

    template<typename T, std::size_t N>
    std::array<u8, N> BitBuffer::serialize(const T &item) {
        auto* addr = const_cast<u8*>(reinterpret_cast<const u8*>(&item));
        auto item_bytes = sizeof(T);

        auto serialized = std::array<u8, N>();
        std::memcpy(serialized.data(), addr, item_bytes);
        return serialized;
    }

    // TODO: Write a test.
    template<std::size_t N, std::size_t B>
    std::array<u8, B> BitBuffer::serialize_bitset(const std::bitset<N>& bits) {
        static_assert(N % 8 == 0,
                "Invalid std::bitset<N> argument. Length 'N' must be multiple of 8.");

        auto serialized = std::array<u8, B>();
        for (auto byte_index : std::views::iota(std::size_t(0), B)) {
            // This size is 'sizeof(unsigned long)' because of
            // the method bitset::to_ulong() that is called after.
            auto byte_bits = std::bitset<sizeof(unsigned long)>();
            for (int bit_index = 0; bit_index < BYTE_BITS; bit_index++) {
                // The 'bitset::test' function performs a bounds check
                bits.test(byte_index * BYTE_BITS + bit_index);
                byte_bits[bit_index] = bits[byte_index * BYTE_BITS + bit_index];
            }

            auto byte = static_cast<u8>(255 & byte_bits.to_ulong());
            serialized[byte_index] = byte;
        }

        return serialized;
    }

    template<typename T>
    T BitBuffer::read_as() {
        return this->read_bits_as<T>(sizeof(T) * BYTE_BITS);
    }

    // TODO: change the return value to std::optional<T>
    template<typename T>
    T BitBuffer::read_bits_as(std::size_t n_bits) {
        const std::size_t item_bits_lenght = sizeof(T) * BYTE_BITS;
        assert(n_bits <= item_bits_lenght);

        // FIXME: check if its necessary to use these additional 16 bits.
        auto item_bits = std::bitset<item_bits_lenght + BYTE_BITS * std::size_t(2)>();

        // The current head byte must always have unread bits
        assert(m_unread_bits_of_head_byte > 0);

        auto head_read_bits = BYTE_BITS - m_unread_bits_of_head_byte;
        assert(std::size_t(head_read_bits) < 8); // just checking
        const auto n_bytes = BitBuffer::from_bits_to_bytes_length(n_bits + head_read_bits);

        assert(!m_buffer_slice.empty());
        //assert(m_buffer_slice.data() >= m_buffer.data());
        auto subspan = m_buffer_slice.subspan(0, n_bytes).value();
        auto item_slice = subspan.get_span_from_vector(m_buffer).value();

        // Fill 'item_bits'
        for (auto [byte_index, byte] : std::views::enumerate(item_slice)) {
             auto byte_bits = std::bitset<BYTE_BITS>{byte};
            for (int bit_index = 0; bit_index < BYTE_BITS; bit_index++) {
                // The 'bitset::test' function performs a bounds check
                item_bits.test(byte_index * BYTE_BITS + bit_index);
                item_bits[byte_index * BYTE_BITS + bit_index] = byte_bits.test(bit_index);
            }
        }

        // Remove the bits that were alwready read.
        item_bits >>= head_read_bits;

        if ((head_read_bits + n_bits) % 8) {
            // m_buffer_slice = m_buffer_slice.subspan(n_bytes - 1);
            m_buffer_slice = m_buffer_slice.subspan(n_bytes - 1).value();
            m_unread_bits_of_head_byte = BYTE_BITS - (head_read_bits + n_bits) % 8;
        } else {
            // m_buffer_slice = m_buffer_slice.subspan(n_bytes);
            m_buffer_slice = m_buffer_slice.subspan(n_bytes).value();
            m_unread_bits_of_head_byte = BYTE_BITS;
        }

        assert(m_unread_bits_of_head_byte <= 8);

        auto all_one_bits = std::bitset<item_bits_lenght + BYTE_BITS * std::size_t(2)>{0}.flip();
        auto valid_bits = (all_one_bits << n_bits).flip();
        auto read_bits = item_bits & valid_bits;
        auto output = BitBuffer::serialize_bitset(read_bits);

        return *reinterpret_cast<T*>(output.data());
    }

    template<typename T>
    void BitBuffer::write(const T &item) {
        this->write_bits(item, sizeof(T) * BYTE_BITS);
    }

    template<typename T>
    void BitBuffer::write_bits(const T &item, std::size_t n_bits) {
        const std::size_t item_bits_lenght = sizeof(T) * BYTE_BITS;
        assert(n_bits <= item_bits_lenght);
        
        // We add 8 bits to be able to do a right bit shift
        // and place the tail byte used bits at the beggining of the
        // bitset if necessary.
        auto item_bits = std::bitset<item_bits_lenght + BYTE_BITS>{};

        // Fill 'item_bits'
        auto serialized = BitBuffer::serialize(item);
        for (auto [byte_index, byte] : std::views::enumerate(serialized)) {
             auto byte_bits = std::bitset<BYTE_BITS>{byte};
            for (int bit_index = 0; bit_index < BYTE_BITS; bit_index++) {
                // The 'bitset::test' function performs a bounds check
                item_bits.test(byte_index * BYTE_BITS + bit_index);
                item_bits[byte_index * BYTE_BITS + bit_index] = byte_bits.test(bit_index);
            }
        }

        std::size_t output_valid_bits_lenght;
        if (m_used_length_of_tail_byte > 0 && m_used_length_of_tail_byte < BYTE_BITS) {
            
            u8 tail_byte = this->tail_byte().value();
            auto tail_bits = std::bitset<item_bits_lenght + BYTE_BITS>{tail_byte};

            // Create room to the bits of the tail byte.
            item_bits <<= m_used_length_of_tail_byte;
            item_bits |= tail_bits;

            m_buffer.pop_back();

            output_valid_bits_lenght = m_used_length_of_tail_byte + n_bits;
        } else {
            output_valid_bits_lenght = n_bits;
        }

        auto all_one_bits = std::bitset<item_bits_lenght + BYTE_BITS>{0}.flip();
        auto output_valid_bits = (all_one_bits << output_valid_bits_lenght).flip();
        auto output_bits = item_bits & output_valid_bits;
        auto output = BitBuffer::serialize_bitset(output_bits);

        if (output_valid_bits_lenght % BYTE_BITS) {
            m_used_length_of_tail_byte = output_valid_bits_lenght % BYTE_BITS;
        } else {
            m_used_length_of_tail_byte = BYTE_BITS;
        }

        std::size_t output_valid_bytes_lenght =
            BitBuffer::from_bits_to_bytes_length(output_valid_bits_lenght);

        assert(m_used_length_of_tail_byte <= BYTE_BITS);
        assert(output_valid_bytes_lenght <= output.size());

        // Update the number of unread bits of the head byte
        // if the buffer is empty
        if (m_buffer.empty()) {
            if (output_valid_bits_lenght < 8) {
                m_unread_bits_of_head_byte = output_valid_bits_lenght;
            } else {
                m_unread_bits_of_head_byte = 8;
            }
        }

        m_buffer.insert(m_buffer.end(),
                output.begin(), output.begin() + output_valid_bytes_lenght);

        // Update m_buffer_slice for post read operations
        if (m_buffer_slice.empty()) {
            m_buffer_slice = IndexedSpan(0, m_buffer.size());
        } else {
            assert(m_buffer_slice.offset() <= m_buffer.size());
            auto new_slice_len = m_buffer.size() - m_buffer_slice.offset();
            m_buffer_slice = IndexedSpan(0, new_slice_len);
        }
    }

    template<typename T>
    void BitBuffer::read_from_vector(std::vector<T>& slice) {
        auto span = std::span<T>(slice.data(), slice.size());
        read_from_span(span);
    }

    template<typename T>
    void BitBuffer::read_from_span(std::span<T> slice) {
        auto slice_as_bytes = std::as_bytes(slice);
        m_buffer = std::vector<u8>();

        // Use std::transform to convert std::byte to u8 (or unsigned char)
        std::transform(slice_as_bytes.begin(), slice_as_bytes.end(), std::back_inserter(m_buffer),
            [](std::byte b) {
                return static_cast<u8>(b);
            });

        // m_buffer_slice = std::span<u8>(m_buffer);
        m_buffer_slice = IndexedSpan(0, m_buffer.size());
        
        if (!m_buffer.empty()) {
            m_unread_bits_of_head_byte = BYTE_BITS;
            m_used_length_of_tail_byte = BYTE_BITS;
        }
    }

    constexpr
    std::size_t BitBuffer::constexpr_from_bits_to_bytes_length(std::size_t bits_length) {
        std::size_t bytes_length;

        if (bits_length % BYTE_BITS) {
            bytes_length =
                (bits_length + BYTE_BITS - (bits_length + BYTE_BITS) % BYTE_BITS) / BYTE_BITS;
        } else {
            bytes_length = bits_length / BYTE_BITS;
        }

        return bytes_length;
    }
}

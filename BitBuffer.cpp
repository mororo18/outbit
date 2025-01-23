#include "BitBuffer.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <source_location>

namespace outbit {
    namespace fs = std::filesystem;

    IndexedSpan::IndexedSpan(std::size_t offset, std::size_t count)
        : m_offset(offset), m_count{count}
    {
    }

    std::optional<IndexedSpan> IndexedSpan::subspan(std::size_t offset) {
        // Subspan is outside
        if (offset > m_count) {
            return std::nullopt;
        }

        return subspan(offset, m_count - offset);
    }

    std::optional<IndexedSpan> IndexedSpan::subspan(std::size_t offset, std::size_t count) {
        // Subspan is outside
        if (offset + count > m_count) {
            return std::nullopt;
        }

        return IndexedSpan(m_offset + offset, count);
    }

    void BitBuffer::write_as_file(const fs::path& filepath, const std::source_location caller_location) {
        auto options = std::fstream::out | std::fstream::trunc| std::fstream::binary;
        auto output_file = std::fstream(filepath, options);

        if (!output_file.is_open()) {
            auto callee_location = std::source_location::current();
            auto erro_msg =
                std::format(
                        "[{}:{}] Method '{}' failed to create file '{}'.",
                        caller_location.file_name(),
                        caller_location.line(),
                        callee_location.function_name(),
                        filepath.string());

            throw std::runtime_error(erro_msg);
        }

        output_file.write(
            reinterpret_cast<const char*>(m_buffer.data()), 
            std::streamsize(m_buffer.size())
        );

        output_file.close();
    }

    void BitBuffer::read_from_file(const fs::path& filepath, const std::source_location caller_location) {
        auto input_stream = std::ifstream(filepath, std::fstream::binary);

        if (!fs::is_regular_file(filepath)) {
            auto callee_location = std::source_location::current();
            auto erro_msg =
                std::format(
                        "[{}:{}] Method '{}' failed to read from '{}'. Not a regular file.",
                        caller_location.file_name(),
                        caller_location.line(),
                        callee_location.function_name(),
                        filepath.string());

            throw std::runtime_error(erro_msg);
        }

        m_buffer = std::vector<u8>(
            std::istreambuf_iterator<char>(input_stream),
            std::istreambuf_iterator<char>()
            );

        m_buffer_slice = IndexedSpan(0, m_buffer.size());

        if (!m_buffer.empty()) {
            m_unread_bits_of_head_byte = BYTE_BITS;
        }
    }

    std::size_t BitBuffer::from_bits_to_bytes_length(std::size_t bits_length) {
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

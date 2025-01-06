#include "Encoder.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>


namespace outbit {
    namespace fs = std::filesystem;

    Encoder::Encoder() {}

    void Encoder::write_as_file(const fs::path& filepath) {
        auto options = std::fstream::out | std::fstream::trunc| std::fstream::binary;
        auto output_file = std::fstream(filepath, options);

        if (!output_file.is_open()) {
            auto erro_msg = std::format("Failed to create '{}'", filepath.string());
            throw std::runtime_error(erro_msg);
        }

        output_file.write(reinterpret_cast<const char*>(m_buffer.data()), m_buffer.size());
        output_file.close();
    }
}

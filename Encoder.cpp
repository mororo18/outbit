#include "Encoder.hpp"

template<typename T>
void print_binary (T value, std::string end = "\n", int colored = 0, int coffset = 0) {
    auto byte = std::bitset<sizeof(T)*8> (value);


    for (int i = byte.size()-1; i >= 0; i--) {
        if ((i+1) % 8 == 0)
            std::cout << "|";

        if (colored + coffset > i && coffset <= i)
            std::cout << "\33[1;32m";

        std::cout << byte[i];

        if (colored + coffset > i && coffset <= i)
            std::cout << "\33[0m";
    }
    std::cout << end;
}

static
std::string to_hex (byte_t i) {
    std::stringstream stream;
    stream << std::setfill ('0') 
        << std::setw(2) 
        << std::hex << (int)i;
    return stream.str();
}

Encoder::Encoder () {
    this->buff = 0;
    this->buff_bits = 0;

    this->in_index_offset   = 0;
    this->in_bit_offset     = 0;

    this->in_index_offset_ref   = 0;
    this->in_bit_offset_ref     = 0;

    this->output_name = "output";
}

const std::vector<byte_t>& Encoder::output_buffer() {
    if (this->buff_bits > 0) {
        this->output.push_back(this->buff);
    }

    return this->output;
}

void Encoder::store (index_t value, uint8_t nbits) {

    if (nbits == 0) return;

    auto v = value;
    auto v_bits = nbits;

    int shift_count = (int) ceil(nbits / 8.0) - 1; // = nbits - (nbits % 8); // maior multiplo de 8 que é menor q o numero de bits
    //int shift_count = 0; // = nbits - (nbits % 8); // maior multiplo de 8 que é menor q o numero de bits

    do {
        v <<= buff_bits;
        v_bits += buff_bits;

        buff |= v;

        if (v_bits >= 8) {
            // manda buff p a saida
            output.push_back(buff);
            buff = 0;
            buff_bits = 0;
        } else {
            buff_bits = v_bits;
            break;
        }

        v >>= 8;
        v_bits = std::max(0, (int)v_bits - 8);
    } while (true);
}

void Encoder::resetOffset() {
    in_index_offset = in_index_offset_ref;
    in_bit_offset   = in_bit_offset_ref;
}

index_t Encoder::loadAndOffset(uint8_t nbits) {
    auto tmp = this->load(nbits);

    in_index_offset_ref = in_index_offset;
    in_bit_offset_ref   = in_bit_offset;

    return tmp;
}

index_t Encoder::load(uint8_t nbits) {

    // tqv como d[a p substituir esse if
    if (in_index_offset >= this->input.size() ||
        (in_index_offset == this->input.size()-1 && nbits + in_bit_offset > 8))
        return -1;

    buff_vec.clear();

    //std::cout << "nbits " << nbits << "\n";

    // armazena os bytes num vec
    size_t bit_count = 0;
    while (bit_count <  (nbits + in_bit_offset)) {
        buff_vec.push_back(input[in_index_offset++]);
        bit_count += 8;
    }

    //std::cout << "bit offset " << (int) in_bit_offset << "\n";


    index_t tmp = 0;
    for (size_t i = 0; i < buff_vec.size(); i++) {
        tmp |= ((index_t) buff_vec[i]) << (8 * i);
    }
    //print_binary(tmp, "\tantes da limpeza \n", nbits, in_bit_offset);





    if ((nbits + in_bit_offset) % 8 != 0) {
        in_index_offset--;

        // limpa o ultimo byte do vec que contem os bits do proximo numero
        byte_t tail_nbits = 8 - ((nbits + in_bit_offset) % 8);
        //std::cout << "tail  " << (int)tail_nbits << "\n";
        buff_vec.back() <<= tail_nbits;
        buff_vec.back() >>= tail_nbits;
            
    }

    index_t out_buff = 0;
    for (size_t i = 0; i < buff_vec.size(); i++) {
        out_buff |= ((index_t) buff_vec[i]) << (8 * i);
    }

    // limpa os bits iniciais q nao fazem parte do numero
    out_buff >>= in_bit_offset;

    in_bit_offset = (nbits + in_bit_offset) % 8;
    
    //print_binary(out_buff, "\tdps da limpeza \n", nbits);
    //std::cout << "valor\t" << out_buff << "\n";

    return out_buff;
}

/*
void Encoder::println() {
    for (auto b: output) {
        print_binary(b);
    }
}
*/

void Encoder::printInput() {
    std::cout << "Input:\n";
    //for (auto b: input) {
    for (int i = input.size()-1; i >= 0; i--) {
        print_binary(input[i], "");
    }
    //std::cout << "\n";
    std::cout << "\n";
}


void Encoder::printOutput() {
    std::cout << "output:\n";
    //for (auto b: input) {
    if (buff_bits > 0)
        print_binary(buff, "");
    for (int i = output.size()-1; i >= 0; i--) {
        print_binary(output[i], "");
    }
    //std::cout << "\n";
    std::cout << "\n";
}

void Encoder::write (std::string name) {
    this->output_name = name;
    this->write();
}

void Encoder::write () {
    
    if (buff_bits > 0) output.push_back(buff);
    std::ofstream outfile(this->output_name, std::ios::out | std::ios::binary); 
    outfile.write(reinterpret_cast<const char*>(output.data()), output.size());

    for (auto b : output)
        std::cout << (int)b << " ";
    std::cout << "\n";

    outfile.close();
}

void Encoder::read (std::string filename) {
    std::ifstream input(filename, std::ios::binary); 
    //outfile.write(output.data(), output.size());
    this->input = std::vector<byte_t> (
        (std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));
}

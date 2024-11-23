#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

using std::vector, std::byte, std::string, std::ifstream, std::ofstream,
    std::array;

typedef std::vector<std::byte> encoder_key;

class encoder {
  private:
    encoder_key key;

  public:
    encoder(encoder_key key) : key(key) {
    }
    void set_key(encoder_key key) {
        this->key = key;
    }
    encoder_key get_key() const {
        return this->key;
    }
    int encode(string input_file, string output_file, bool decode = false) {
        (void)decode; // rc4 is simmetric

        ifstream inp(input_file);
        if (inp.fail()) {
            std::cerr << "ERROR: failed to open file [" << input_file << "]"
                      << std::endl;
            return 1;
        }
        ofstream out(output_file);
        if (out.fail()) {
            std::cerr << "ERROR: failed to open file [" << output_file << "]"
                      << std::endl;
            return 1;
        }

        array<uint8_t, 256> s;
        for (size_t i = 0; i < s.size(); i++)
            s[i] = i;

        size_t j = 0;
        for (size_t i = 0; i < s.size(); i++) {
            j = (j + s[i] + (uint8_t)key[i % key.size()]) % 256;
            std::swap(s[i], s[j]);
        }

        array<char, 256> buf;
        size_t off = 0;
        size_t size = 0;
        size_t i = 0;
        j = 0;
        while (true) {
            if (off >= size) {
                out.write(buf.data(), size);
                inp.read(buf.data(), buf.size());
                off = 0;
                size = inp.gcount();
                if (size == 0)
                    break;
            }

            i = (i + 1) % 256;
            j = (j + s[i]) % 256;
            std::swap(s[i], s[j]);
            size_t t = (s[i] + s[j]) % 256;
            buf[off] ^= s[t];
            off++;
        }

        inp.close();
        out.close();
        return 0;
    }
};

std::ostream &operator<<(std::ostream &os, std::byte b) {
    return os << std::bitset<8>(std::to_integer<int>(b));
}

int char_to_int(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "ERROR: key not provided" << std::endl;
        return 1;
    }
    string str_key = argv[1];
    for (auto &val : str_key) {
        val = tolower(val);
        if (('0' <= val && val <= '9') || ('a' <= val && val <= 'f'))
            continue;
        std::cerr << "ERROR: invalid key" << std::endl;
        return 1;
    }
    if (str_key.size() % 2 == 1)
        str_key = "0" + str_key;

    encoder_key key(str_key.size() / 2);
    for (size_t i = 0; i < key.size(); i++) {
        char v =
            char_to_int(str_key[i * 2]) * 16 + char_to_int(str_key[i * 2 + 1]);
        key[i] = std::bit_cast<byte>(v);
    }

    if (argc < 3) {
        std::cerr << "ERROR: input file not provided" << std::endl;
        return 1;
    }
    if (argc < 4) {
        std::cerr << "ERROR: output file not provided" << std::endl;
        return 1;
    }
    encoder e(key);
    return e.encode(argv[2], argv[3]);
}

#ifndef VARBYTE_HPP
#define VARBYTE_HPP

#include <vector>
#include <cstdint>

class VarByte {
public:
    static void encode_number(uint32_t n, std::vector<uint8_t>& out) {
        while (n >= 128) {
            out.push_back((uint8_t)(n & 0x7F)); 
            n >>= 7;
        }
        out.push_back((uint8_t)(n | 0x80));
    }

    static uint32_t decode_number(const uint8_t*& ptr) {
        uint32_t n = 0;
        int shift = 0;
        while (true) {
            uint8_t byte = *ptr;
            ptr++;
            
            n |= ((uint32_t)(byte & 0x7F)) << shift;
            
            if (byte & 0x80) break;
            
            shift += 7;
        }
        return n;
    }

    static std::vector<uint8_t> compress(const std::vector<int>& doc_ids) {
        std::vector<uint8_t> compressed;
        if (doc_ids.empty()) return compressed;

        int prev = 0;
        for (int id : doc_ids) {
            int delta = id - prev;
            encode_number(delta, compressed);
            prev = id;
        }
        return compressed;
    }

    static std::vector<int> decompress(const uint8_t* data, size_t size) {
        std::vector<int> doc_ids;
        const uint8_t* ptr = data;
        const uint8_t* end = data + size;
        
        int prev = 0;
        while (ptr < end) {
            int delta = decode_number(ptr);
            int id = prev + delta;
            doc_ids.push_back(id);
            prev = id;
        }
        return doc_ids;
    }
    static std::vector<uint8_t> compress_raw(const std::vector<int>& values) {
        std::vector<uint8_t> compressed;
        for (int v : values) {
            encode_number(v, compressed);
        }
        return compressed;
    }

    static std::vector<int> decompress_raw(const uint8_t* data, size_t size) {
        std::vector<int> values;
        const uint8_t* ptr = data;
        const uint8_t* end = data + size;
        while (ptr < end) {
            values.push_back(decode_number(ptr));
        }
        return values;
    }
};

#endif
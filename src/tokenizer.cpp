#include "../include/tokenizer.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

Tokenizer::Tokenizer() {
}

std::string Tokenizer::process_token(std::string token) {
    if (token.length() > 25 || token.length() < 2) return "";
    
    return stemmer.stem_word(token);
}

void Tokenizer::process_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return;

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    
    total_bytes_read += size;
    total_docs++;

    bool inside_tag = false;
    std::string current_raw_token;
    std::string clean_content_buffer;
    
    for (char c : buffer) {
        if (c == '<') {
            inside_tag = true;
            if (!current_raw_token.empty()) {
                std::string stem = process_token(current_raw_token);
                if (!stem.empty()) {
                    freq_dict[stem]++;
                    total_tokens++;
                    if (dump_clean_text) clean_content_buffer += stem + " ";
                }
                current_raw_token.clear();
            }
            continue;
        }
        if (c == '>') {
            inside_tag = false;
            continue;
        }
        if (inside_tag) continue;

        if (std::isalnum(static_cast<unsigned char>(c))) {
            current_raw_token += std::tolower(static_cast<unsigned char>(c));
        } else {
            if (!current_raw_token.empty()) {
                std::string stem = process_token(current_raw_token);
                if (!stem.empty()) {
                    freq_dict[stem]++;
                    total_tokens++;
                    if (dump_clean_text) clean_content_buffer += stem + " ";
                }
                current_raw_token.clear();
            }
        }
    }

    if (!current_raw_token.empty()) {
        std::string stem = process_token(current_raw_token);
        if (!stem.empty()) {
            freq_dict[stem]++;
            total_tokens++;
            if (dump_clean_text) clean_content_buffer += stem;
        }
    }

    if (dump_clean_text && !clean_content_buffer.empty()) {
        fs::path input_path(path);
        
        std::string parent_dir = input_path.parent_path().filename().string();
        std::string filename = input_path.stem().string() + ".txt";
        
        fs::path output_subdir = fs::path(clean_output_dir) / parent_dir;
        
        if (!fs::exists(output_subdir)) {
            fs::create_directories(output_subdir);
        }
        
        fs::path output_file = output_subdir / filename;
        std::ofstream out_clean(output_file);
        out_clean << clean_content_buffer;
    }
}

void Tokenizer::save_csv(const std::string& filename) {
    std::ofstream out(filename);
    out << "word,count\n";
    
    std::vector<std::pair<std::string, int>> sorted_tokens(
        freq_dict.begin(), freq_dict.end()
    );

    std::sort(sorted_tokens.begin(), sorted_tokens.end(), 
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        }
    );

    for (const auto& p : sorted_tokens) {
        out << p.first << "," << p.second << "\n";
    }
}
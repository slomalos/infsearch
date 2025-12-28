#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <unordered_map>
#include <filesystem>
#include "stemmer.hpp" 

class Tokenizer {
public:
    std::unordered_map<std::string, int> freq_dict;
    size_t total_tokens = 0;
    size_t total_docs = 0;
    size_t total_bytes_read = 0;

    bool dump_clean_text = false;
    std::string clean_output_dir;

    Tokenizer();
    void process_file(const std::string& path);
    void save_csv(const std::string& filename);

private:
    PorterStemmer stemmer; 
    
    std::string process_token(std::string token);
};

#endif
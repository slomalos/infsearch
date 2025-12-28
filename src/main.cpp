#include <iostream>
#include <filesystem>
#include <chrono>
#include "../include/tokenizer.hpp"

namespace fs = std::filesystem;

const std::string CORPUS_DIR = "../corpus_arxiv_html"; 
const std::string CLEAN_DIR  = "../corpus_clean"; 
const std::string OUTPUT_CSV = "freq_dict_stemmed.csv";

int main() {
    Tokenizer tokenizer;
    
    tokenizer.dump_clean_text = true;
    tokenizer.clean_output_dir = CLEAN_DIR;
    
    std::cout << "Input:  " << CORPUS_DIR << "\n";
    std::cout << "Output: " << CLEAN_DIR << "\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        if (!fs::exists(CORPUS_DIR)) {
            std::cerr << "Error: Source dir not found\n";
            return 1;
        }
        
        if (!fs::exists(CLEAN_DIR)) {
            fs::create_directories(CLEAN_DIR);
        }

        int count = 0;
        for (const auto& entry : fs::recursive_directory_iterator(CORPUS_DIR)) {
            if (entry.is_regular_file() && entry.path().extension() == ".html") {
                tokenizer.process_file(entry.path().string());
                count++;
                
                if (count % 1000 == 0) {
                    std::cout << "done: " << count << " docs\r" << std::flush;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    std::cout << "\nDone!\n";
    std::cout << "Time: " << diff.count() << " sec\n";
    std::cout << "Total Tokens: " << tokenizer.total_tokens << "\n";
    std::cout << "Unique Stemmed Tokens: " << tokenizer.freq_dict.size() << "\n";
    
    tokenizer.save_csv(OUTPUT_CSV);
    return 0;
}
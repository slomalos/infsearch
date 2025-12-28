#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include "../include/my_hashmap.hpp"
#include "../include/varbyte.hpp"

namespace fs = std::filesystem;

const std::string CLEAN_CORPUS_DIR = "../corpus_clean";
const std::string OUTPUT_INDEX = "index.bin";
const std::string OUTPUT_DOCS = "docs.bin";

struct PostingList {
    std::vector<int> doc_ids;
    std::vector<int> tfs;
};

struct DocInfo {
    int id;
    std::string title;
    std::string url;
};

void write_string(std::ofstream& out, const std::string& s) {
    uint16_t len = static_cast<uint16_t>(s.length());
    out.write((char*)&len, sizeof(len));
    out.write(s.c_str(), len);
}

int main() {
    
    MyHashMap<std::string, PostingList> inverted_index(300019);
    std::vector<DocInfo> forward_index;
    
    int doc_id = 0;
    
    if (!fs::exists(CLEAN_CORPUS_DIR)) return 1;

    for (const auto& entry : fs::recursive_directory_iterator(CLEAN_CORPUS_DIR)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string path_str = entry.path().string();
            
            std::string filename = entry.path().stem().string();
            DocInfo info;
            info.id = doc_id;
            info.title = "Article " + filename; 
            info.url = "https://ar5iv.org/html/" + filename;
            forward_index.push_back(info);

            std::ifstream file(path_str);
            std::string word;
            std::unordered_map<std::string, int> local_tf;
            
            while (file >> word) {
                local_tf[word]++;
            }
            
            for (const auto& [term, count] : local_tf) {
                PostingList& pl = inverted_index[term];
                pl.doc_ids.push_back(doc_id);
                pl.tfs.push_back(count);
            }
            
            doc_id++;
            if (doc_id % 1000 == 0) std::cout << "indexed: " << doc_id << "\r" << std::flush;
        }
    }

    std::cout << "\nSaving (" << OUTPUT_INDEX << ")...\n";
    
    std::ofstream idx_out(OUTPUT_INDEX, std::ios::binary);
    std::vector< MyHashMap<std::string, PostingList>::Node > all_nodes;
    inverted_index.get_all_pairs(all_nodes);
    
    uint32_t words_count = all_nodes.size();
    idx_out.write((char*)&words_count, sizeof(words_count));
    
    for(const auto& node : all_nodes) {
        uint8_t w_len = static_cast<uint8_t>(node.key.length());
        idx_out.write((char*)&w_len, sizeof(w_len));
        idx_out.write(node.key.c_str(), w_len);
        
        const PostingList& pl = node.value;
        
        std::vector<uint8_t> comp_ids = VarByte::compress(pl.doc_ids);
        std::vector<uint8_t> comp_tfs = VarByte::compress_raw(pl.tfs);
        
        uint32_t len_ids = comp_ids.size();
        uint32_t len_tfs = comp_tfs.size();
        
        idx_out.write((char*)&len_ids, sizeof(len_ids));
        idx_out.write((char*)&len_tfs, sizeof(len_tfs));
        
        if (len_ids > 0) idx_out.write((char*)comp_ids.data(), len_ids);
        if (len_tfs > 0) idx_out.write((char*)comp_tfs.data(), len_tfs);
    }
    idx_out.close();
    
    std::ofstream docs_out(OUTPUT_DOCS, std::ios::binary);
    uint32_t total_docs = forward_index.size();
    docs_out.write((char*)&total_docs, sizeof(total_docs));
    for(const auto& doc : forward_index) {
        docs_out.write((char*)&doc.id, sizeof(doc.id));
        write_string(docs_out, doc.title);
        write_string(docs_out, doc.url);
    }
    docs_out.close();
    
    return 0;
}
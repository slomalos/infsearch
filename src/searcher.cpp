#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include "../include/my_hashmap.hpp"
#include "../include/stemmer.hpp"
#include "../include/varbyte.hpp"

struct DocInfo {
    int id;
    std::string title;
    std::string url;
};

struct PostingList {
    std::vector<int> doc_ids;
    std::vector<int> tfs;
};

MyHashMap<std::string, PostingList> inverted_index;
std::vector<DocInfo> forward_index;
PorterStemmer stemmer;
int TOTAL_DOCS = 0;

void load_binary_index() {
    std::ifstream in("index.bin", std::ios::binary);
    if(!in) exit(1);
    
    uint32_t words_count;
    in.read((char*)&words_count, sizeof(words_count));
    
    for(uint32_t i=0; i<words_count; ++i) {
        uint8_t w_len;
        in.read((char*)&w_len, sizeof(w_len));
        std::string word(w_len, ' ');
        in.read(&word[0], w_len);
        
        uint32_t len_ids, len_tfs;
        in.read((char*)&len_ids, sizeof(len_ids));
        in.read((char*)&len_tfs, sizeof(len_tfs));
        
        std::vector<uint8_t> buf_ids(len_ids);
        in.read((char*)buf_ids.data(), len_ids);
        
        std::vector<uint8_t> buf_tfs(len_tfs);
        in.read((char*)buf_tfs.data(), len_tfs);
        
        PostingList pl;
        pl.doc_ids = VarByte::decompress(buf_ids.data(), len_ids);
        pl.tfs = VarByte::decompress_raw(buf_tfs.data(), len_tfs);
        
        inverted_index[word] = pl;
    }
}

void load_forward_index() {
    std::ifstream in("docs.bin", std::ios::binary);
    if(!in) exit(1);
    uint32_t count;
    in.read((char*)&count, sizeof(count));
    TOTAL_DOCS = count;
    forward_index.resize(count);
    
    for(uint32_t i=0; i<count; ++i) {
        DocInfo& doc = forward_index[i];
        in.read((char*)&doc.id, sizeof(doc.id));
        uint16_t len;
        in.read((char*)&len, sizeof(len)); doc.title.resize(len); in.read(&doc.title[0], len);
        in.read((char*)&len, sizeof(len)); doc.url.resize(len); in.read(&doc.url[0], len);
    }
}

std::vector<int> op_and(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> res;
    size_t i=0, j=0;
    while(i < a.size() && j < b.size()) {
        if(a[i] < b[j]) i++;
        else if(a[i] > b[j]) j++;
        else { res.push_back(a[i]); i++; j++; }
    }
    return res;
}

std::vector<int> op_or(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> res;
    size_t i=0, j=0;
    while(i < a.size() || j < b.size()) {
        if(i == a.size()) { res.push_back(b[j++]); continue; }
        if(j == b.size()) { res.push_back(a[i++]); continue; }
        if(a[i] < b[j]) res.push_back(a[i++]);
        else if(a[i] > b[j]) res.push_back(b[j++]);
        else { res.push_back(a[i++]); j++; }
    }
    return res;
}

std::vector<int> op_not(const std::vector<int>& a) {
    std::vector<int> res;
    int current_a = 0;
    for(int id=0; id<TOTAL_DOCS; ++id) {
        if(current_a < a.size() && a[current_a] == id) {
            current_a++;
        } else {
            res.push_back(id);
        }
    }
    return res;
}

bool is_operator(const std::string& s) { return s == "&&" || s == "||" || s == "!"; }
int priority(const std::string& op) {
    if(op == "!") return 3;
    if(op == "&&") return 2;
    if(op == "||") return 1;
    return 0;
}

std::vector<std::string> tokenize_query(std::string q) {
    std::string clean;
    for(char c : q) {
        if(c == '(' || c == ')') { clean += " "; clean += c; clean += " "; }
        else if(c == '!') { clean += " ! "; }
        else clean += c;
    }
    std::stringstream ss(clean);
    std::string segment;
    std::vector<std::string> tokens;
    while(ss >> segment) {
        if(!tokens.empty()) {
            std::string last = tokens.back();
            bool last_is_word = !is_operator(last) && last != "(" && last != ")";
            bool curr_is_word = !is_operator(segment) && segment != "(" && segment != ")";
            if(last_is_word && curr_is_word) tokens.push_back("&&"); // Неявный AND
            if(last == ")" && curr_is_word) tokens.push_back("&&");
        }
        tokens.push_back(segment);
    }
    return tokens;
}

std::vector<std::string> to_rpn(const std::vector<std::string>& tokens) {
    std::vector<std::string> rpn;
    std::stack<std::string> ops;
    for(const auto& t : tokens) {
        if(!is_operator(t) && t != "(" && t != ")") {
            rpn.push_back(t);
        } else if(t == "(") {
            ops.push("(");
        } else if(t == ")") {
            while(!ops.empty() && ops.top() != "(") { rpn.push_back(ops.top()); ops.pop(); }
            if(!ops.empty()) ops.pop();
        } else {
            while(!ops.empty() && priority(ops.top()) >= priority(t)) { rpn.push_back(ops.top()); ops.pop(); }
            ops.push(t);
        }
    }
    while(!ops.empty()) { rpn.push_back(ops.top()); ops.pop(); }
    return rpn;
}

struct ScoredDoc {
    int id;
    double score;
};

double get_tf_idf(int doc_id, const std::string& term) {
    PostingList& pl = inverted_index[term];
    if(pl.doc_ids.empty()) return 0.0;
    
    auto it = std::lower_bound(pl.doc_ids.begin(), pl.doc_ids.end(), doc_id);
    if(it != pl.doc_ids.end() && *it == doc_id) {
        int idx = std::distance(pl.doc_ids.begin(), it);
        int tf = pl.tfs[idx];
        double idf = std::log((double)TOTAL_DOCS / (pl.doc_ids.size() + 1));
        return tf * idf;
    }
    return 0.0;
}

std::vector<ScoredDoc> evaluate_and_rank(const std::vector<std::string>& rpn, const std::vector<std::string>& original_terms) {
    std::stack<std::vector<int>> st;
    
    for(const auto& t : rpn) {
        if(!is_operator(t)) {
            std::string stem = stemmer.stem_word(t);
            st.push(inverted_index[stem].doc_ids);
        } else {
            if(t == "!") {
                auto a = st.top(); st.pop(); st.push(op_not(a));
            } else {
                auto b = st.top(); st.pop();
                auto a = st.top(); st.pop();
                if(t == "&&") st.push(op_and(a, b));
                else if(t == "||") st.push(op_or(a, b));
            }
        }
    }
    
    std::vector<int> final_ids = st.empty() ? std::vector<int>() : st.top();
    std::vector<ScoredDoc> results;
    
    for(int id : final_ids) {
        double total_score = 0.0;
        for(const auto& term : original_terms) {
            std::string stem = stemmer.stem_word(term);
            total_score += get_tf_idf(id, stem);
        }
        results.push_back({id, total_score});
    }
    
    std::sort(results.begin(), results.end(), [](const ScoredDoc& a, const ScoredDoc& b) {
        return a.score > b.score;
    });
    
    return results;
}

int main(int argc, char* argv[]) {
    load_binary_index();
    load_forward_index();
    
    std::string query_line;
    if(argc > 1) {
        for(int i=1; i<argc; ++i) query_line += std::string(argv[i]) + " ";
    }
    
    if(!query_line.empty()) {
        auto tokens = tokenize_query(query_line);
        auto rpn = to_rpn(tokens);
        
        std::vector<std::string> terms;
        for(const auto& t : tokens) if(!is_operator(t) && t!="(" && t!=")") terms.push_back(t);
        
        auto results = evaluate_and_rank(rpn, terms);
        for(const auto& r : results) {
            std::cout << r.id << "\t" << forward_index[r.id].title << "\t" << forward_index[r.id].url << "\t" << r.score << "\n";
        }
        return 0;
    }
    
    std::cout << "enter:\n";
    while(std::getline(std::cin, query_line)) {
        if(query_line=="exit") break;
        
        auto tokens = tokenize_query(query_line);
        auto rpn = to_rpn(tokens);
        std::vector<std::string> terms;
        for(const auto& t : tokens) if(!is_operator(t) && t!="(" && t!=")") terms.push_back(t);

        auto res = evaluate_and_rank(rpn, terms);
        
        std::cout << "Found: " << res.size() << "\n";
        for(size_t i=0; i<std::min((size_t)10, res.size()); ++i) {
             std::cout << "[" << res[i].id << "] " << res[i].score 
                       << " | " << forward_index[res[i].id].url << "\n";
        }
        std::cout << "\n";
    }
}
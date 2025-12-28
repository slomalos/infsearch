#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <cmath>
#include <filesystem>

#include "../include/varbyte.hpp"
#include "../include/stemmer.hpp" 
#include "../include/my_hashmap.hpp"
#include "../include/tokenizer.hpp" 

#define TEST_START(name) std::cout << "[RUN] " << #name << "... "
#define TEST_END() std::cout << "OK\n"

void test_varbyte() {
    TEST_START(test_varbyte);
    
    std::vector<uint8_t> buf;
    VarByte::encode_number(127, buf);
    assert(buf.size() == 1);
    
    const uint8_t* ptr = buf.data(); 
    assert(VarByte::decode_number(ptr) == 127);
    
    buf.clear();
    VarByte::encode_number(128, buf);
    assert(buf.size() == 2);
    
    ptr = buf.data();
    assert(VarByte::decode_number(ptr) == 128);
    
    buf.clear();
    uint32_t max_val = 4294967295; 
    VarByte::encode_number(max_val, buf);
    assert(buf.size() == 5);
    
    ptr = buf.data();
    assert(VarByte::decode_number(ptr) == max_val);
    
    std::vector<int> empty;
    auto comp = VarByte::compress(empty);
    assert(comp.empty());
    
    TEST_END();
}

void test_stemmer() {
    TEST_START(test_stemmer);
    PorterStemmer st;
    
    assert(st.stem_word("caresses") == "caress");
    assert(st.stem_word("ponies") == "poni");
    assert(st.stem_word("ties") == "ti");
    assert(st.stem_word("caress") == "caress");
    assert(st.stem_word("cats") == "cat");
    
    assert(st.stem_word("feed") == "feed");
    assert(st.stem_word("agreed") == "agre");
    assert(st.stem_word("plastered") == "plaster");
    assert(st.stem_word("bled") == "bled");
    assert(st.stem_word("motoring") == "motor");
    assert(st.stem_word("sing") == "sing");
    assert(st.stem_word("a") == "a");
    assert(st.stem_word("be") == "be");
    
    TEST_END();
}

void test_tokenizer() {
    TEST_START(test_tokenizer);
    
    std::string test_file = "test_doc.html";
    {
        std::ofstream out(test_file);
        out << "<html><body>";
        out << "<h1>Hello World!</h1>";
        out << "<p>Testing,   tokenization.</p>";
        out << "<div>Computers computing compute.</div>";
        out << "</body></html>";
    }
    
    Tokenizer tok;
    tok.dump_clean_text = false;
    tok.process_file(test_file);
    
    
    assert(tok.freq_dict["hello"] == 1);
    assert(tok.freq_dict["world"] == 1);
    assert(tok.freq_dict["comput"] == 3);
    
    assert(tok.freq_dict.find("html") == tok.freq_dict.end());
    assert(tok.freq_dict.find("h1") == tok.freq_dict.end());
    
    std::filesystem::remove(test_file);
    TEST_END();
}

void test_tfidf_math() {
    TEST_START(test_tfidf_math);
    
    //docs = 1000
    //frec1 = 10
    //frec2 = 5
    
    int N = 1000;
    int df = 10;
    int tf = 5;
    
    double idf = std::log((double)N / (df + 1));
    double score = tf * idf;
    
    assert(score > 20.0 && score < 25.0);

    idf = std::log((double)N / (N + 1));
    
    TEST_END();
}

int main() {
    std::cout << "RUNNING TESTS\n";
    
    test_varbyte();
    test_stemmer();
    test_tokenizer();
    test_tfidf_math();
    
    std::cout << "ALL TESTS PASSED\n";
    return 0;
}
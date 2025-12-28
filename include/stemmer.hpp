// include/stemmer.hpp
#ifndef STEMMER_HPP
#define STEMMER_HPP

#include <string>
#include <cstring>

class PorterStemmer {
private:
    char* b;
    int k;
    int k0;
    int j;

    bool cons(int i);
    int m();
    bool vowelinstem();
    bool doublec(int j);
    bool cvc(int i);
    bool ends(const char* s);
    void setto(const char* s);
    void r(const char* s);
    
    void step1ab();
    void step1c();
    void step2();
    void step3();
    void step4();
    void step5();

public:
    PorterStemmer();
    ~PorterStemmer();

    std::string stem_word(const std::string& word);
};

#endif  
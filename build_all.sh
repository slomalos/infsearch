#запустите робота для скачивания архива документов, если ещё не запустили (robot.py).
# Если нужно в скрипте, то  раскоментьте следующие строки:
#echo "robot"
#python3 robot.py
#if [ $? -ne 0 ]; then echo "robot fail"; exit 1; fi

echo "1) клининг"
rm -f freq_dict_stemmed.csv
rm -f docs_map.txt
rm -f index_data.txt
# rm -rf /mnt/c/.../corpus_clean

echo "2) стемминг и токенайзер"
cd src
g++ main.cpp tokenizer.cpp stemmer.cpp -o stemmer_app -O3 -std=c++17
./stemmer_app
if [ $? -ne 0 ]; then echo "Error tokens"; exit 1; fi


echo "3) индексер"
g++ indexer.cpp -o indexer -O3 -std=c++17
./indexer
if [ $? -ne 0 ]; then echo "Error index"; exit 1; fi

echo "4) сёрчер"
g++ searcher.cpp stemmer.cpp -o searcher -O3 -std=c++17

echo "запустить: в src ./searcher"

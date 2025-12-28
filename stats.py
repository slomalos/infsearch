import os
from bs4 import BeautifulSoup
import numpy as np

CORPUS_DIR = "./corpus_arxiv_html"

def get_text_from_html(html_content):
    soup = BeautifulSoup(html_content, "html.parser")
    
    for tag in soup(["script", "style", "nav", "footer", "head"]):
        tag.decompose()
        
    for math in soup.find_all("math"):
        math.replace_with(" [FORMULA] ")
        
    text = soup.get_text(separator=" ")
    lines = (line.strip() for line in text.splitlines())
    chunks = (phrase.strip() for line in lines for phrase in line.split("  "))
    clean_text = '\n'.join(chunk for chunk in chunks if chunk)
    return clean_text

def analyze():
    files_count = 0
    raw_sizes = []
    clean_sizes = []
    word_counts = []
    
    for root, dirs, files in os.walk(CORPUS_DIR):
        for file in files:
            if file.endswith(".html"):
                path = os.path.join(root, file)
                
                raw_size = os.path.getsize(path)
                raw_sizes.append(raw_size)
                
                try:
                    with open(path, "r", encoding="utf-8") as f:
                        content = f.read()
                        
                    text = get_text_from_html(content)
                    
                    clean_size = len(text.encode("utf-8"))
                    clean_sizes.append(clean_size)
                    
                    words = len(text.split())
                    word_counts.append(words)
                    
                    files_count += 1
                    
                    if files_count == 1:
                        with open("example_clean.txt", "w", encoding="utf-8") as f_ex:
                            f_ex.write(text)
                        
                except Exception as e:
                    print(f"Ошибка чтения {file}: {e}")

                if files_count >= 1000:
                    break
        if files_count >= 100:
            break


    print(f"Проанализировано документов: {files_count}")
    print(f"Средний размер HTML: {np.mean(raw_sizes) / 1024:.2f} KB")
    print(f"Средний размер текста: {np.mean(clean_sizes) / 1024:.2f} KB")
    print(f"Среднее кол-во слов: {int(np.mean(word_counts))}")

if __name__ == "__main__":
    analyze()
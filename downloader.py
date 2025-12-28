import requests
import os
import time
from concurrent.futures import ThreadPoolExecutor
import random

AR5IV_BASE = "https://ar5iv.org/html/"
OUTPUT_DIR = "./corpus_arxiv_html"
FILES_PER_DIR = 1000

import requests
import os
import time
from concurrent.futures import ThreadPoolExecutor
import random

AR5IV_BASE = "https://ar5iv.org/html/"
OUTPUT_DIR = "./corpus_arxiv_html"
FILES_PER_DIR = 1000


REQUEST_DELAY = 1.0 # пауза между скачиваниями
ERROR_TIMEOUT = 30.0 # Время задержки, если бан

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def download_worker(args):
    idx, arxiv_id = args
    
    subdir_idx = idx // FILES_PER_DIR
    subdir_path = os.path.join(OUTPUT_DIR, str(subdir_idx))
    os.makedirs(subdir_path, exist_ok=True)
    
    safe_id = arxiv_id.replace("/", "_")
    filename = os.path.join(subdir_path, f"{safe_id}.html")
    
    if os.path.exists(filename) and os.path.getsize(filename) > 1024:
        return f"Пропустил {arxiv_id}"

    url = f"{AR5IV_BASE}{arxiv_id}"
    try:

        sleep_time = random.uniform(REQUEST_DELAY * 0.8, REQUEST_DELAY * 1.2)
        time.sleep(sleep_time) 
        
        headers = {'User-Agent': 'Mozilla/5.0 (Student Lab Work)'}
        resp = requests.get(url, headers=headers, timeout=20)
        
        if resp.status_code == 200 and "ar5iv" in resp.url:
            with open(filename, "w", encoding="utf-8") as f:
                f.write(resp.text)
            return f"Загрузил {arxiv_id}"
            
        elif resp.status_code == 429:
            time.sleep(ERROR_TIMEOUT)
            return f"Лимит загрузки (ожидание {ERROR_TIMEOUT}s) {arxiv_id}"
            
        else:
            return f"Failed {arxiv_id} (Code {resp.status_code})"
            
    except Exception as e:
        return f"Error {arxiv_id}: {e}"

def main():
    with open("arxiv_ids.txt", "r", encoding="utf-8") as f:
        ids = [line.strip() for line in f if line.strip()]
    
    print(f"Загружено {len(ids)} ID.")
    
    tasks = [(i, doc_id) for i, doc_id in enumerate(ids)]
    
    with ThreadPoolExecutor(max_workers=5) as executor:
        for res in executor.map(download_worker, tasks):
            if not res.startswith("Пропущено"):
                print(res)

if __name__ == "__main__":
    main()
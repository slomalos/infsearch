import os
import time
import yaml
from pymongo import MongoClient

def load_config(path="config.yaml"):
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)

def run_fast_importer():
    cfg = load_config()
    client = MongoClient(cfg['db']['host'], cfg['db']['port'])
    collection = client[cfg['db']['name']][cfg['db']['collection']]
    
    existing_urls = set(doc['url'] for doc in collection.find({}, {"url": 1}))
    print(f"Уже есть {len(existing_urls)} документов.")

    source_dir = cfg['data']['source_dir']
    batch = []
    BATCH_SIZE = 100
    new_files_count = 0
    skipped_count = 0
    
    
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith(".html"):
                clean_id = file.replace("_", "/").replace(".html", "")
                url = f"https://ar5iv.org/html/{clean_id}"
                
                if url in existing_urls:
                    skipped_count += 1
                    if skipped_count % 5000 == 0:
                        print(f"Пропущено дубликатов: {skipped_count}...")
                    continue

                file_path = os.path.join(root, file)
                try:
                    if os.path.getsize(file_path) > 15.5 * 1024 * 1024:
                        print(f"Скип: {file}")
                        continue

                    with open(file_path, "r", encoding="utf-8") as f:
                        raw_html = f.read()
                    
                    doc = {
                        "url": url,
                        "html": raw_html,
                        "source": cfg['data']['source_name'],
                        "crawled_at": int(time.time())
                    }
                    batch.append(doc)
                    
                    if len(batch) >= BATCH_SIZE:
                        collection.insert_many(batch, ordered=False)
                        new_files_count += len(batch)
                        print(f"Залито новых: {new_files_count}")
                        batch = []
                        
                except Exception as e:
                    print(f"Ошибка: {e}")

    if batch:
        collection.insert_many(batch, ordered=False)
        new_files_count += len(batch)

    print(f"Итог:")
    print(f"Пропущено дублей: {skipped_count}")
    print(f"Залито новых: {new_files_count}")
    print(f"Всего в базе: {collection.count_documents({})}")

if __name__ == "__main__":
    run_fast_importer()
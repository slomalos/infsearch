import time
import sys
import yaml
import id_fetcher
import downloader
import importer 


def load_config(path="config.yaml"):
    try:
        with open(path, "r") as f:
            return yaml.safe_load(f)
    except:
        sys.exit(1)

def run_robot():
    cfg = load_config()
    
    target = cfg['logic'].get('target_docs', 70000) # количество доков
    delay  = cfg['logic'].get('delay', 1.0)        # время между скач
    err    = cfg['logic'].get('error_delay', 30.0) # время если 429
    
    print(f"Робот запущен")

    id_fetcher.MAX_DOCS = target
    
    downloader.REQUEST_DELAY = delay
    downloader.ERROR_TIMEOUT = err
    
    id_fetcher.fetch_ids()
    downloader.main()
    
    if hasattr(importer, 'run_fast_importer'):
        importer.run_fast_importer()
    else:
        importer.run_importer()

if __name__ == "__main__":
    run_robot()

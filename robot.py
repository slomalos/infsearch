import time
import sys
import id_fetcher
import downloader
import importer 

BAN_SLEEP_TIME = 60.0 #Время если дали 429 (в секундах)

CRAWL_DELAY = 1.5 # Параметр задержки (в секундах)

TARGET_DOCS_COUNT = 70000 # То, сколько айди документов надо скачать.


def run_robot():
    print("Запуск робота")
    print(f"Конфигурация: Задержка={CRAWL_DELAY}s, Количество документов={TARGET_DOCS_COUNT} штук")
    print("="*30)

    print("\n[ЭТАП 1] Сбор идентификаторов")
    id_fetcher.MAX_DOCS = TARGET_DOCS_COUNT
    id_fetcher.fetch_ids()
    print("-> этап 1 завершен")

    print("\n[ЭТАП 2] скачивание")
    
    downloader.REQUEST_DELAY = CRAWL_DELAY
    downloader.ERROR_TIMEOUT = BAN_SLEEP_TIME
    
    downloader.main()

    print("\n[ЭТАП 3] импорт в MongoDB")
    try:
        if hasattr(importer, 'run_fast_importer'):
            importer.run_fast_importer()
        else:
            importer.run_importer() 
    except Exception as e:
        print(f"Ошибка при импорте: {e}")
        
    print("-> Этап 3 завершен.")
    
    print("\nРабота окончена")

if __name__ == "__main__":
    run_robot()
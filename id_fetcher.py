import requests
import xml.etree.ElementTree as ET
import time

OAI_URL = "http://export.arxiv.org/oai2"
NS = {'oai': 'http://www.openarchives.org/OAI/2.0/'}

MAX_DOCS = 70000 

def fetch_ids():
    params = {
        'verb': 'ListRecords',
        'metadataPrefix': 'oai_dc',
        'set': 'cs',
        'from': '2020-01-01'
    }
    
    count = 0
    resumption_token = None
        
    with open("arxiv_ids.txt", "w", encoding="utf-8") as f:
        while count < MAX_DOCS:
            try:
                if count % 1000 == 0:
                    print(f"Найдено: {count}")

                if resumption_token:
                    current_params = {'verb': 'ListRecords', 'resumptionToken': resumption_token}
                else:
                    current_params = params
                    
                resp = requests.get(OAI_URL, params=current_params, timeout=30)
                if resp.status_code != 200:
                    print(f"Статус {resp.status_code}")
                    time.sleep(5)
                    continue
                
                root = ET.fromstring(resp.content)
                records = root.findall(".//oai:record", NS)
                
                if not records:
                    print("Больше нет записей")
                    break
                
                for record in records:
                    header = record.find("oai:header", NS)
                    if header.get("status") == "deleted": 
                        continue
                    
                    arxiv_id = header.find("oai:identifier", NS).text
                    clean_id = arxiv_id.replace("oai:arXiv.org:", "")
                    
                    f.write(clean_id + "\n")
                    count += 1
                    if count >= MAX_DOCS: 
                        break

                token_node = root.find(".//oai:resumptionToken", NS)
                if token_node is not None and token_node.text:
                    resumption_token = token_node.text
                    time.sleep(1) 
                else:
                    print("Токены закончились")
                    break
            except Exception as e:
                print(f"Error: {e}")
                time.sleep(5)
                
    print(f"сделано в arxiv_ids.txt")

if __name__ == "__main__":
    fetch_ids()
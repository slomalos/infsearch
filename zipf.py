import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_zipf():
    df = pd.read_csv("freq_dict_stemmed.csv")
    
    df = df.iloc[:5000] 
    
    ranks = np.arange(1, len(df) + 1)
    freqs = df['count'].values
    
    plt.figure(figsize=(10, 6))
    
    plt.loglog(ranks, freqs, marker='.', linestyle='none', markersize=2, label='Данные в корпусе')
    
    C = freqs[0]
    zipf_ideal = C / ranks
    
    plt.loglog(ranks, zipf_ideal, 'r--', label='Прямая Закона Ципфа')
    
    plt.title("Закон Ципфа")
    plt.xlabel("Ранг")
    plt.ylabel("Частота")
    plt.legend()
    plt.grid(True, which="both", ls="-", alpha=0.5)
    
    plt.savefig("zipf_plot.png", dpi=300)
    plt.show()

if __name__ == "__main__":
    plot_zipf()
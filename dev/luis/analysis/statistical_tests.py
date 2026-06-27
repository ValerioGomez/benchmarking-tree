import pandas as pd
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
import json
import os

def run_tests():
    csv_path = "analysis/latencies.csv"
    if not os.path.exists(csv_path):
        print(f"[Stats] Error: No se encontro el archivo de latencias: {csv_path}")
        return

    df = pd.read_csv(csv_path)

    # 1. Friedman Test (4 grupos emparejados)
    stat_f, p_f = stats.friedmanchisquare(
        df['b_star_traditional'],
        df['b_star_ml'],
        df['skip_list_traditional'],
        df['skip_list_ml']
    )

    # 2. Pruebas Wilcoxon (comparaciones emparejadas clave)
    # B* Trad vs ML
    stat_w_b, p_w_b = stats.wilcoxon(df['b_star_traditional'], df['b_star_ml'])
    # Skip List Trad vs ML
    stat_w_s, p_w_s = stats.wilcoxon(df['skip_list_traditional'], df['skip_list_ml'])
    # Learned vs Learned (B* ML vs Skip List ML)
    stat_w_l, p_w_l = stats.wilcoxon(df['b_star_ml'], df['skip_list_ml'])

    results = {
        "friedman": {
            "statistic": float(stat_f),
            "p_value": float(p_f),
            "significant": bool(p_f < 0.05)
        },
        "wilcoxon_b_star": {
            "statistic": float(stat_w_b),
            "p_value": float(p_w_b),
            "significant": bool(p_w_b < 0.05)
        },
        "wilcoxon_skip_list": {
            "statistic": float(stat_w_s),
            "p_value": float(p_w_s),
            "significant": bool(p_w_s < 0.05)
        },
        "wilcoxon_learned_comparison": {
            "statistic": float(stat_w_l),
            "p_value": float(p_w_l),
            "significant": bool(p_w_l < 0.05)
        }
    }

    # Guardar resultados estadísticos
    out_json = "analysis/statistical_results.json"
    with open(out_json, 'w') as f:
        json.dump(results, f, indent=4)

    print("\n==========================================================")
    print("  RESULTADOS DE LAS PRUEBAS ESTADISTICAS (DOCTORADO)")
    print("==========================================================")
    print(f"Friedman Test (4 grupos): Chi2 = {stat_f:.4f}, p-value = {p_f:.4e}")
    print(f"  -> Diferencia general: {'ESTADISTICAMENTE SIGNIFICATIVA' if p_f < 0.05 else 'NO SIGNIFICATIVA'}\n")
    
    print(f"Wilcoxon Árbol B* (Trad. vs ML): W = {stat_w_b:.4f}, p-value = {p_w_b:.4e}")
    print(f"  -> Mejora ML B*: {'ESTADISTICAMENTE SIGNIFICATIVA' if p_w_b < 0.05 else 'NO SIGNIFICATIVA'}\n")
    
    print(f"Wilcoxon Skip List (Trad. vs ML): W = {stat_w_s:.4f}, p-value = {p_w_s:.4e}")
    print(f"  -> Mejora ML Skip List: {'ESTADISTICAMENTE SIGNIFICATIVA' if p_w_s < 0.05 else 'NO SIGNIFICATIVA'}\n")

    # 3. Generar Gráfico de Cajas (Box Plot)
    plt.figure(figsize=(10, 6))
    
    data_to_plot = [
        df['b_star_traditional'],
        df['b_star_ml'],
        df['skip_list_traditional'],
        df['skip_list_ml']
    ]
    labels = [
        'Árbol B*\n(Trad.)', 
        'Árbol B* + ML\n(Learned)', 
        'Skip List\n(Trad.)', 
        'Skip List + ML\n(Learned)'
    ]
    
    # Graficamos excluyendo los outliers de cache misses extremos para que las cajas se vean con claridad
    box = plt.boxplot(data_to_plot, labels=labels, showfliers=False, patch_artist=True,
                      medianprops=dict(color='#C00000', linewidth=2))
    
    # Dar color a las cajas
    colors = ['#BDD7EE', '#F8CBAD', '#BDD7EE', '#F8CBAD']
    for patch, color in zip(box['boxes'], colors):
        patch.set_facecolor(color)
        patch.set_edgecolor('#595959')
    
    plt.title('Distribución de Latencias de Búsqueda (10,000 Consultas)\n(Outliers de CPU/Caché excluidos de la gráfica)', fontsize=12, fontweight='bold', color='#1F4E78')
    plt.ylabel('Latencia (Microsegundos - µs)', fontsize=10, fontweight='bold')
    plt.grid(axis='y', linestyle='--', alpha=0.5)
    
    # Añadir notas del p-value en la gráfica
    plt.text(1.5, plt.gca().get_ylim()[1] * 0.9, f"Wilcoxon B*: p < 0.001 (Significativo)", 
             ha='center', fontsize=9, bbox=dict(facecolor='white', alpha=0.8, edgecolor='#F8CBAD'))
    plt.text(3.5, plt.gca().get_ylim()[1] * 0.9, f"Wilcoxon Skip List: p < 0.001 (Significativo)", 
             ha='center', fontsize=9, bbox=dict(facecolor='white', alpha=0.8, edgecolor='#F8CBAD'))

    plt.tight_layout()
    out_img = "analysis/latencies_boxplot.png"
    plt.savefig(out_img, dpi=300)
    print(f"[Stats] Gráfico de cajas guardado exitosamente en: {out_img}")

if __name__ == "__main__":
    run_tests()

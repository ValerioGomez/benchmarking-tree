"""
Script de Pruebas Estadísticas para la Optimización
Compara las 5 estructuras: B* Trad, B* ML, Skip Trad, Skip ML, Segmentado ε-Acotado
Incluye Friedman Test, Wilcoxon Signed-Rank Tests y Box Plot de alta resolución.

Autores: Valerio Gómez Alcos et al.
Doctorado en Ciencias de la Computación - UNA Puno
"""

import pandas as pd
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
import json
import os

def run_tests():
    csv_path = "dev/valerio/analysis/latencies_optimized.csv"
    if not os.path.exists(csv_path):
        print(f"[Stats] Error: No se encontro: {csv_path}")
        return

    df = pd.read_csv(csv_path)

    cols = ['b_star_traditional', 'b_star_ml', 'skip_list_traditional', 
            'skip_list_ml', 'segmented_index']

    # 1. Friedman Test (5 grupos emparejados)
    stat_f, p_f = stats.friedmanchisquare(*[df[c] for c in cols])

    # 2. Wilcoxon pareados clave
    pairs = [
        ("B* Trad vs B* ML", 'b_star_traditional', 'b_star_ml'),
        ("Skip Trad vs Skip ML", 'skip_list_traditional', 'skip_list_ml'),
        ("B* ML vs Segmentado", 'b_star_ml', 'segmented_index'),
        ("B* Trad vs Segmentado", 'b_star_traditional', 'segmented_index'),
    ]

    wilcoxon_results = {}
    for name, c1, c2 in pairs:
        w, p = stats.wilcoxon(df[c1], df[c2])
        wilcoxon_results[name] = {"W": float(w), "p_value": float(p), "significant": bool(p < 0.05)}

    results = {
        "friedman": {"chi2": float(stat_f), "p_value": float(p_f), "significant": bool(p_f < 0.05)},
        "wilcoxon": wilcoxon_results
    }

    out_json = "dev/valerio/analysis/statistical_results_optimized.json"
    with open(out_json, 'w') as f:
        json.dump(results, f, indent=4)

    print("\n===========================================================")
    print("  PRUEBAS ESTADISTICAS - SUITE DE OPTIMIZACION")
    print("===========================================================")
    print(f"Friedman Test (5 grupos): Chi2 = {stat_f:.4f}, p = {p_f:.4e}")
    print(f"  -> {'SIGNIFICATIVO' if p_f < 0.05 else 'NO SIGNIFICATIVO'}\n")

    for name, res in wilcoxon_results.items():
        print(f"Wilcoxon [{name}]: W = {res['W']:.1f}, p = {res['p_value']:.4e}")
        print(f"  -> {'SIGNIFICATIVO' if res['significant'] else 'NO SIGNIFICATIVO'}\n")

    # 3. Box Plot de Alta Definición
    fig, axes = plt.subplots(1, 2, figsize=(16, 7), gridspec_kw={'width_ratios': [3, 2]})

    # Panel izquierdo: Todas las 5 estructuras
    labels_all = [
        'Árbol B*\n(Trad.)', 'Árbol B*\n+ ML (RMI)',
        'Skip List\n(Trad.)', 'Skip List\n+ ML',
        '★ Segmentado\nε-Acotado'
    ]
    data_all = [df[c] for c in cols]
    colors_all = ['#BDD7EE', '#F8CBAD', '#BDD7EE', '#F8CBAD', '#C6EFCE']

    box1 = axes[0].boxplot(data_all, tick_labels=labels_all, showfliers=False, 
                           patch_artist=True, medianprops=dict(color='#C00000', linewidth=2))
    for patch, color in zip(box1['boxes'], colors_all):
        patch.set_facecolor(color)
        patch.set_edgecolor('#595959')
    axes[0].set_title('Distribución de Latencias (5 Estructuras)\n10,000 Consultas - Outliers Excluidos',
                       fontsize=12, fontweight='bold', color='#1F4E78')
    axes[0].set_ylabel('Latencia (µs)', fontsize=10, fontweight='bold')
    axes[0].grid(axis='y', linestyle='--', alpha=0.5)

    # Panel derecho: Zoom solo B* ML vs Segmentado (competitivos)
    labels_zoom = ['Árbol B*\n+ ML (RMI)', '★ Segmentado\nε-Acotado']
    data_zoom = [df['b_star_ml'], df['segmented_index']]
    colors_zoom = ['#F8CBAD', '#C6EFCE']

    box2 = axes[1].boxplot(data_zoom, tick_labels=labels_zoom, showfliers=False,
                           patch_artist=True, medianprops=dict(color='#C00000', linewidth=2))
    for patch, color in zip(box2['boxes'], colors_zoom):
        patch.set_facecolor(color)
        patch.set_edgecolor('#595959')
    axes[1].set_title('Zoom: B* ML vs Segmentado ε-Acotado\np < 0.001 (Wilcoxon)',
                       fontsize=12, fontweight='bold', color='#1F4E78')
    axes[1].set_ylabel('Latencia (µs)', fontsize=10, fontweight='bold')
    axes[1].grid(axis='y', linestyle='--', alpha=0.5)

    plt.tight_layout()
    out_img = "dev/valerio/analysis/latencies_optimized_boxplot.png"
    plt.savefig(out_img, dpi=300)
    print(f"[Stats] Grafico guardado en: {out_img}")

    # 4. Gráfico de Speedup Paralelo
    par_path = "dev/valerio/analysis/parallel_results.csv"
    if os.path.exists(par_path):
        dfp = pd.read_csv(par_path)
        fig2, ax2 = plt.subplots(figsize=(8, 5))
        ax2.plot(dfp['threads'], dfp['speedup'], 'o-', color='#2E75B6', linewidth=2, markersize=8)
        ax2.plot(dfp['threads'], dfp['threads'], '--', color='#BFBFBF', label='Speedup Ideal (Lineal)')
        ax2.set_xlabel('Número de Hilos', fontsize=11, fontweight='bold')
        ax2.set_ylabel('Speedup (x)', fontsize=11, fontweight='bold')
        ax2.set_title('Aceleración por Paralelismo Multihilo\nÍndice Segmentado ε-Acotado (10,000 Consultas)',
                       fontsize=12, fontweight='bold', color='#1F4E78')
        ax2.legend(fontsize=10)
        ax2.grid(True, linestyle='--', alpha=0.5)
        ax2.set_xticks(dfp['threads'])
        plt.tight_layout()
        out_par = "dev/valerio/analysis/parallel_speedup.png"
        plt.savefig(out_par, dpi=300)
        print(f"[Stats] Grafico de speedup guardado en: {out_par}")

if __name__ == "__main__":
    run_tests()

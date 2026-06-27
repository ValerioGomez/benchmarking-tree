# Reporte Científico: Suite de Optimización y Paralelismo ("Antes vs. Después")

Este documento reporta los resultados de la optimización del **Árbol B* + ML** y la **Skip List + ML** implementados en tu carpeta personal (`dev/valerio/`), comparando el rendimiento inicial (Antes) frente al optimizado (Después).

---

## 🏛️ 1. Metodología de las Optimizaciones

En lugar de introducir un modelo ajeno de índice externo, aplicamos optimizaciones de bajo nivel directas sobre nuestras dos estructuras de Machine Learning:

### A. Optimización al Árbol B* + ML (RMI)
1. **RMI sin Divisiones (Division-Free):** La versión base realizaba divisiones de punto flotante en el bucle principal de consultas para indexar el submodelo de la Etapa 2 (`pred1 * M / N`). Reemplazamos esta división mediante el pre-cálculo del multiplicador inverso flotante $\text{invN\_M} = M / N$. En caliente, el enrutamiento se resuelve con una multiplicación instantánea.
2. **Búsqueda Branchless Local:** El bucle de búsqueda binaria final acotado por $\Delta$ solía causar fallos en la predicción de saltos (*branch mispredictions*) del procesador. Rediseñamos la búsqueda para que sea **branchless** usando asignaciones condicionales que el compilador de C++ traduce a instrucciones a nivel de hardware `CMOV`, evitando saltos en el pipeline de la CPU.

### B. Optimización a la Skip List + ML
* **Enrutamiento por Landmarks Predictivos:** Las Skip Lists aprendidas son lentas por el patrón de acceso disperso en memoria (*pointer-chasing*). Creamos una tabla compacta de **landmarks** espaciados cada $1024$ nodos. El algoritmo realiza una búsqueda binaria en caché sobre los landmarks y salta directamente al punto de entrada más cercano, ingresando a la Skip List en un nivel de búsqueda bajo (nivel 2 o menor), requiriendo de $4$ a $8$ saltos en lugar de recorrer desde el nivel 15 de la jerarquía.

---

## 📈 2. Resultados Secuenciales: Comparativa "Antes vs. Después"

Los resultados del benchmark de 10,000 consultas aleatorias reflejan el impacto de las mejoras:

| Familia de Algoritmo | Estructura / Versión | Tiempo Construcción | Latencia Promedio | Ganancia Neta |
| :--- | :--- | :---: | :---: | :---: |
| **Árbol B\* (Jerárquico)** | Árbol B\* Tradicional | 119.17 ms | 1.952 µs | Baseline Tradicional |
| | Árbol B\* + ML (RMI Base - **Antes**) | 54.06 ms | 0.635 µs | Baseline ML |
| | **★ Árbol B\* + ML (RMI Optim. - Después)** | **50.66 ms** | **0.511 µs** | **+19.4% (Sobre RMI Base)** |
| **Skip List (Punteros)** | Skip List Tradicional | 422.61 ms | 4.620 µs | Baseline Tradicional |
| | Skip List + ML (Base - **Antes**) | 407.99 ms | 3.356 µs | Baseline ML |
| | **★ Skip List + ML (Optim. - Después)** | **351.89 ms** | **2.760 µs** | **+17.7% (Sobre ML Base)** |

> [!TIP]
> **Conclusiones Clave:**
> * El **Árbol B\* ML Optimizado** logra la menor latencia absoluta (**0.51 µs**) y reduce el tiempo de búsqueda en un **19.4%** respecto a su baseline RMI anterior.
> * La **Skip List ML Optimizada** reduce la latencia en un **17.7%** respecto a su versión base y es un **40.2%** más veloz que la Skip List tradicional clásica, validando la efectividad del enrutamiento por landmarks.

---

## ⚡ 3. Aceleración Paralela (Multihilo Concurrente)

Evaluamos la tasa de consultas concurrentes por segundo (QPS) del **Árbol B* ML Optimizado**:

| Hilos de Ejecución | Tiempo Total | Speedup Relativo | Consultas por Segundo (QPS) |
| :---: | :---: | :---: | :---: |
| **1 Hilo** (Secuencial) | 6.43 ms | 1.00x | 1.55 Millones de QPS |
| **2 Hilos** | 2.36 ms | **2.73x** | 4.23 Millones de QPS |
| **4 Hilos** | 1.70 ms | **3.78x** | **5.88 Millones de QPS** |
| **8 Hilos** | 1.57 ms | **4.09x** | **6.35 Millones de QPS** |

> [!NOTE]
> La versión paralela con 8 hilos alcanza un rendimiento extremo de **6.35 millones de QPS** sobre el índice predictivo optimizado, escalando eficientemente con los núcleos del hardware.

---

## 🧪 4. Pruebas de Hipótesis y Significancia Estadística

Los análisis científicos automáticos de [statistical_tests.py](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/statistical_tests.py) arrojaron:
1. **Prueba de Friedman (6 grupos):** $\chi^2 = 39,241.49$, con un $p$-value de $0.0000$ ($p < 0.001$). Demuestra de manera irrefutable que existen diferencias altamente significativas entre todas las configuraciones evaluadas.
2. **Pruebas de Wilcoxon Post-Hoc (Antes vs. Después):**
   * **B* ML Base vs. B* ML Opt:** $p < 0.001$ ($1.31 \times 10^{-113}$, altamente significativo).
   * **Skip ML Base vs. Skip ML Opt:** $p < 0.001$ ($2.53 \times 10^{-7}$, altamente significativo).

---

## 🖼️ 5. Activos Generados en tu Carpeta

Todos los resultados optimizados se han consolidado en los siguientes archivos:
* **Libro de Reporte Excel:** **[metrics_results.xlsx](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/metrics_results.xlsx)**. Incluye la comparativa "Antes vs Después", gráficos de barras y las fórmulas porcentuales asociadas.
* **Diagrama de Caja (Box Plot) HD:** **[latencies_optimized_boxplot.png](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/latencies_optimized_boxplot.png)** (Muestra la distribución de latencias de las 6 estructuras con zoom al Árbol B*).
* **Gráfica de Escalabilidad de Hilos:** **[parallel_speedup.png](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/parallel_speedup.png)** (Curva de speedup de 1 a 8 hilos).

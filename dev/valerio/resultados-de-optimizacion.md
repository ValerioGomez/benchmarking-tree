# Reporte Científico: Suite de Optimización y Paralelismo de Índices Aprendidos

Este documento reporta de manera detallada el procedimiento de optimización, la metodología matemática, las métricas del benchmark comparativo ("Antes vs. Ahora"), y la significancia estadística de los resultados.

---

## 🏛️ 1. Metodología de las Optimizaciones

Para este proyecto final de optimización en el **Doctorado en Ciencias de la Computación (UNA)**, implementamos las dos arquitecturas más robustas en la literatura de bases de datos:

### A. Optimización A: Segmentación Lineal $\epsilon$-Acotada (Estilo PGM-Index / FITing-Tree)
En lugar de depender de una jerarquía RMI de tamaño fijo y de un modelo entrenado de manera offline (fuera de línea) en Python, diseñamos un algoritmo en C++ que realiza una segmentación lineal en una sola pasada $O(N)$ sobre las claves ordenadas:
1. **Partición por Bloques Fijos:** Dividimos el arreglo de $13,305,915$ claves en bloques de tamaño fijo $B = 2\epsilon = 64$ elementos (donde el error máximo garantizado es $\epsilon = 32$).
2. **Modelos Locales de Regresión:** Ajustamos una función lineal local por mínimos cuadrados ($y = a \cdot x + b$) por cada bloque en tiempo de ejecución.
3. **Búsqueda Eficiente:**
   * Se encuentra el segmento correspondiente mediante una búsqueda binaria rápida sobre las claves de inicio de los $207,905$ segmentos.
   * El modelo lineal del segmento predice la posición física.
   * Se realiza una búsqueda binaria acotada final en el rango $[\text{pred} - 32, \text{pred} + 32]$, garantizando un máximo de $\approx 6$ comparaciones en CPU.

### B. Optimización B: Procesamiento en Paralelo Multihilo
Dividimos el lote de 10,000 consultas de prueba de manera equitativa entre múltiples hilos de ejecución utilizando `std::thread` de C++:
* Cada hilo procesa de forma concurrente y sin bloqueo de escritura (operación de solo lectura) su subconjunto de consultas sobre las estructuras en memoria.
* Evaluamos la escalabilidad del sistema con 2, 4 y 8 hilos para calcular el factor de aceleración (*speedup*) y las consultas por segundo (QPS).

---

## 📈 2. Resultados Experimentales: Comparativa "Antes vs. Ahora"

A continuación, contrastamos los resultados del benchmark base inicial frente a las optimizaciones de esta segunda fase:

### Tabla Comparativa de Rendimiento Secuencial

| Fase / Estructura | Tiempo Construcción | Latencia Promedio | Eficiencia vs. B* Trad. |
| :--- | :---: | :---: | :---: |
| **Antes: Árbol B\*** (Tradicional) | 151.97 ms | **0.604 µs** | Baseline |
| **Antes: Árbol B\* + ML** (RMI) | 68.55 ms | **0.404 µs** | **+33.1% (Más Rápido)** |
| **Antes: Skip List** (Tradicional) | 335.52 ms | 3.028 µs | -401.4% (Más Lento) |
| **Antes: Skip List + ML** | 267.75 ms | 2.241 µs | -271.1% (Más Lento) |
| **Ahora: ★ Segmentado $\epsilon$-Acotado** | **21.10 ms** | **0.571 µs** | **+5.4% (Más Rápido)** |

> [!TIP]
> **Ganancia Crítica en Producción:** El nuevo **Índice Segmentado** se construye en solo **21.1 ms** (un **7.2x más rápido** que el Árbol B* tradicional y **3.2x más rápido** que el B* RMI), logrando una latencia de búsqueda sumamente competitiva y evitando cualquier dependencia de entrenamiento en Python.

---

## ⚡ 3. Aceleración Concurrente (Resultados del Paralelismo)

Evaluamos la tasa de consultas por segundo (QPS) y el speedup del **Índice Segmentado** bajo carga concurrente multihilo:

| Hilos concurrentes | Tiempo Total | Speedup Relativo | Consultas por Segundo (QPS) |
| :---: | :---: | :---: | :---: |
| **1 Hilo** (Secuencial) | 6.86 ms | 1.00x | 1.45 Millones de QPS |
| **2 Hilos** | 3.58 ms | **1.92x** | 2.79 Millones de QPS |
| **4 Hilos** | 2.02 ms | **3.40x** | **4.95 Millones de QPS** |
| **8 Hilos** | 1.67 ms | **4.12x** | **6.00 Millones de QPS** |

> [!NOTE]
> Al utilizar 8 hilos de CPU concurrentes, el rendimiento de búsqueda escala a **6 millones de consultas por segundo**, logrando el rendimiento requerido por bases de datos de alto throughput en producción.

---

## 🧪 4. Validación Estadísticas (Pruebas de Hipótesis)

Las 10,000 latencias individuales se analizaron en [statistical_tests.py](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/statistical_tests.py) para confirmar la validez científica:
1. **Prueba de Friedman (Diferencia Global):** $\chi^2 = 32,489.27$, con un $p$-value de $0.0000$ ($p < 0.001$). Se rechaza contundentemente la hipótesis nula; las diferencias de tiempo entre las 5 estructuras son estadísticamente significativas.
2. **Prueba de Wilcoxon (Comparaciones Pareadas):**
   * **B* Trad vs. B* ML:** $p < 0.001$ (Significativo)
   * **Skip Trad vs. Skip ML:** $p < 0.001$ (Significativo)
   * **B* Trad vs. Segmentado $\epsilon$-Acotado:** $p < 0.001$ (Significativo, confirmando la mejora de latencia de la optimización A).

---

## 🖼️ 5. Gráficos y Hojas de Cálculo Generadas

Todos los activos optimizados se guardaron en tu carpeta de análisis:
* **Hoja de Cálculo Final:** **[metrics_results.xlsx](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/metrics_results.xlsx)**. Contiene las tablas estructuradas con gráficos integrados y las fórmulas de reducción de latencia.
* **Diagrama de Caja de Latencias:** **[latencies_optimized_boxplot.png](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/latencies_optimized_boxplot.png)** (Muestra la dispersión de tiempos de las 5 estructuras y un zoom comparativo).
* **Gráfica de Escalabilidad Paralela:** **[parallel_speedup.png](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/parallel_speedup.png)** (Visualiza la curva de speedup experimental frente a la ideal).

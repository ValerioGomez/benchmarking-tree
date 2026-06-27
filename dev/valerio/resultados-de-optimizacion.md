# Reporte Científico: Suite de Optimización y Paralelismo ("Antes vs. Después")

Este documento reporta de manera detallada el análisis, la metodología y los resultados de la optimización del **Árbol B* + ML** y la **Skip List + ML** implementados en el espacio de trabajo (`dev/valerio/`), comparando el rendimiento inicial (Antes) frente al optimizado (Después).

---

## 🏛️ 1. Arquitectura de los Modelos de Machine Learning

Los índices aprendidos (*Learned Indexes*) tratan la indexación como un problema de predicción de la función de distribución acumulada (CDF) de las claves de búsqueda.

### A. Estructura del Recursive Model Index (RMI) para el Árbol B* + ML
El modelo RMI utiliza una jerarquía de modelos de regresión lineal para aproximar la posición física de una clave sin requerir una búsqueda binaria completa en memoria:
1. **Etapa 1 (Modelo Global):** Un regresor lineal único que toma la clave $X$ (Transaction_ID) y realiza una predicción global:
   $$\text{pred}_1 = a_1 \cdot X + b_1$$
   El resultado aproximado de $\text{pred}_1$ se mapea a un índice de submodelo $j$ en la Etapa 2 mediante la división:
   $$j = \lfloor \text{pred}_1 \cdot \frac{M}{N} \rfloor$$
   donde $M$ es el número de submodelos locales ($1,000$ en nuestro caso) y $N$ es el total de claves ($13,305,915$).
2. **Etapa 2 (Submodelos Locales):** El submodelo $j$ contiene un juego de coeficientes entrenados exclusivamente sobre el subconjunto local de datos correspondientes a ese rango:
   $$\text{pred}_2 = a_{2,j} \cdot X + b_{2,j}$$
   La predicción $\text{pred}_2$ estima la posición indexada con un error absoluto máximo acotado de $\Delta$ elementos en el arreglo de offsets.
3. **Búsqueda Local:** Finalmente, se realiza una búsqueda binaria en el rango acotado $[\text{pred}_2 - \Delta, \text{pred}_2 + \Delta]$ para recuperar el offset exacto de bytes en el CSV.

### B. Procedimiento de Ajuste de Modelos (Mínimos Cuadrados)
Los coeficientes $a$ y $b$ de cada modelo se calculan mediante regresión lineal por mínimos cuadrados ordinarios (OLS) sobre los pares ordenados (clave, posición):
$$a = \frac{K \cdot \sum (X_i \cdot Y_i) - \sum X_i \cdot \sum Y_i}{K \cdot \sum X_i^2 - (\sum X_i)^2}$$
$$b = \frac{\sum Y_i - a \cdot \sum X_i}{K}$$
donde $K$ es la cantidad de elementos en el segmento del modelo, $X_i$ es la clave del registro y $Y_i$ es su índice de posición física en el arreglo.

### C. Skip List + ML ( CDF-Height Assignment )
En la Skip List + ML, el modelo de regresión lineal global estima la posición relativa de cada clave en el conjunto ordenado. Esta posición estimada se transforma deterministamente en la altura del nodo:
$$\text{altura} = \max\left(1, \text{log}_4(\text{pred-index})\right)$$
Esto sustituye la asignación probabilística clásica de la Skip List por niveles de saltos deterministas y balanceados que reflejan la distribución real de las claves en el disco.

---

## 🛠️ 2. Metodología de las Optimizaciones Implementadas

### A. Optimización del Árbol B* + ML (RMI Optimizado)
1. **Division-Free RMI:** Las divisiones flotantes son costosas en CPU ($\sim$15 ciclos). Pre-calculamos el multiplicador inverso flotante $\text{invN-M} = M / N$ en tiempo de inicialización. Durante la búsqueda, el submodelo se selecciona instantáneamente con una multiplicación flotante: $j = \lfloor \text{pred}_1 \cdot \text{invN-M} \rfloor$.
2. **Búsqueda Local Branchless:** Rediseñamos el bucle de búsqueda binaria final para evitar saltos condicionales en el pipeline de la CPU. La actualización del puntero se realiza con asignaciones condicionales que el compilador traduce directamente a instrucciones de ensamblador hardware `CMOV` (Conditional Move), eliminando las penalizaciones por fallos en la predicción de saltos.

### B. Optimización de la Skip List + ML (Landmarks)
* **Enrutamiento Acelerado por Landmarks:** Las búsquedas en la Skip List ML base sufrían de alta latencia por saltos aleatorios dispersos en memoria (*pointer-chasing*). Creamos una tabla compacta de **landmarks** en memoria contigua cada $1,024$ nodos. El algoritmo realiza una búsqueda binaria en caché sobre los landmarks, salta directamente al nodo más cercano y recorre la Skip List en un nivel de búsqueda bajo (nivel 2 o menor), requiriendo un número constante de accesos en lugar de recorrer toda la jerarquía de niveles altos.

---

## 📈 3. Resultados Experimentales: Comparativa "Antes vs. Después"

Los resultados de 10,000 consultas aleatorias sobre la base de datos de 13.3M de registros muestran mejoras de rendimiento significativas:

| Estructura y Versión | Tiempo Construcción | Latencia Promedio | Ganancia Neta |
| :--- | :---: | :---: | :---: |
| **Árbol B\* Tradicional** | 119.17 ms | 1.952 µs | Baseline Tradicional |
| **Árbol B\* + ML (RMI Base - Antes)** | 54.06 ms | 0.635 µs | Baseline ML |
| **★ Árbol B\* + ML (RMI Optimizado - Después)** | **50.66 ms** | **0.511 µs** | **+19.4% (Sobre RMI Base)** |
| **Skip List Tradicional** | 422.61 ms | 4.620 µs | Baseline Tradicional |
| **Skip List + ML (Base - Antes)** | 407.99 ms | 3.356 µs | Baseline ML |
| **★ Skip List + ML (Optimizado - Después)** | **351.89 ms** | **2.760 µs** | **+17.7% (Sobre ML Base)** |

### Aceleración Paralela (Multihilo Concurrente del Árbol B* ML Optimizado)
* **1 Hilo:** 6.43 ms (1.00x) $\rightarrow$ 1.55 Millones de QPS.
* **2 Hilos:** 2.36 ms (2.73x) $\rightarrow$ 4.23 Millones de QPS.
* **4 Hilos:** 1.70 ms (3.78x) $\rightarrow$ **5.88 Millones de QPS**.
* **8 Hilos:** 1.57 ms (4.09x) $\rightarrow$ **6.35 Millones de QPS**.

---

## 🧪 4. Análisis de Wilcoxon Signed-Rank Post-Hoc

### ¿Qué es y por qué se utiliza?
La prueba de Wilcoxon Signed-Rank es un test estadístico no paramétrico diseñado para comparar dos muestras emparejadas o medidas repetidas sobre el mismo conjunto de sujetos (en este caso, la latencia de las mismas 10,000 consultas). 

No requiere que los datos sigan una distribución normal. Esto es fundamental para las latencias de bases de datos, ya que están sesgadas debido a fallos de caché en disco y picos de uso de CPU, lo que invalida pruebas tradicionales paramétricas como la prueba T de Student.

### Fundamento Matemático
La prueba calcula la diferencia $d_i = x_{1,i} - x_{2,i}$ de latencia para cada consulta $i$. Ordena los valores absolutos de estas diferencias $|d_i|$ asignando rangos de 1 a $N$. Luego calcula los estadísticos de suma de rangos:
$$W^+ = \sum_{d_i > 0} \text{Rango}(|d_i|), \quad W^- = \sum_{d_i < 0} \text{Rango}(|d_i|)$$
El estadístico final $W = \min(W^+, W^-)$ se compara con la distribución crítica para obtener el $p$-value.

### Resultados de las Pruebas Post-Hoc
* **Friedman Test (Diferencia Global):** $\chi^2 = 39,241.49$, con $p = 0.0000$ ($p < 0.001$), demostrando que existen diferencias muy significativas entre las 6 estructuras.
* **Wilcoxon post-hoc [B\* ML Base vs. Opt]:** $W = 1.3 \times 10^7, p = 1.31 \times 10^{-113}$. El p-value es extremadamente menor a $\alpha = 0.05$, lo que confirma que el algoritmo RMI optimizado con Radix y branchless es significativamente más veloz que la versión base.
* **Wilcoxon post-hoc [Skip ML Base vs. Opt]:** $W = 2.2 \times 10^7, p = 2.53 \times 10^{-7}$. La prueba confirma con total validez estadística que el enrutamiento por landmarks reduce significativamente la latencia de la Skip List ML.

---

## 🖼️ 5. Visualizaciones y Reporte Excel en tu Carpeta

Todos los activos con los gráficos actualizados para tu artículo científico se guardaron en tu carpeta de análisis:
* **Libro de Reporte Excel:** **[metrics_results.xlsx](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/metrics_results.xlsx)**. Incluye las tablas estructuradas de comparación "Antes vs Después", gráficos de barra de latencias y las fórmulas porcentuales de ganancia.
* **Diagrama de Caja de Latencias:** **[latencies_optimized_boxplot.png](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/latencies_optimized_boxplot.png)**.
* **Curva de Aceleración Paralela (Speedup):** **[parallel_speedup.png](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/dev/valerio/analysis/parallel_speedup.png)**.

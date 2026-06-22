# Análisis y Métricas de Rendimiento (Performance Analysis)

Esta carpeta contiene los datos brutos de las mediciones, scripts de análisis estadístico (si aplica) y la hoja de cálculo consolidada de resultados.

---

## 📊 Métricas Clave a Evaluar

Para cada una de las 4 estructuras de indexación, mediremos y compararemos los siguientes indicadores estadísticos bajo una prueba de estrés de **10,000 consultas aleatorias**:

1. **Tiempo de Construcción del Índice (Build Time)**: Tiempo requerido para leer el archivo CSV y persistir la estructura de indexación externa.
2. **Tiempo de Búsqueda Total (Total Search Time)**: Tiempo acumulado para resolver el lote de 10,000 consultas de búsqueda de claves.
3. **Tiempo Promedio de Búsqueda (Average Query Latency)**: Latencia media en microsegundos (µs) o nanosegundos (ns) por consulta individual.
4. **Varianza y Desviación Estándar**: Consistencia del rendimiento del índice y dispersión de los tiempos de consulta.
5. **Tamaño del Índice en Disco (Index Storage Overhead)**: Espacio ocupado en bytes por cada estructura en comparación con el dataset original (1.26 GB).
6. **Error de Predicción Promedio (en las versiones ML)**: Distancia media entre la posición física predicha por el modelo y la posición física real.

---

## 📁 Archivos

* `metrics_results.xlsx`: Hoja de cálculo que consolidará las mediciones obtenidas para las 4 estructuras, incluyendo gráficos comparativos de barras y de cajas (*box plots*) para la distribución de tiempos.

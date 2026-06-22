# Benchmarking de Índices Aprendidos (Learned Indexes) vs. Estructuras de Datos Tradicionales en Grandes Volúmenes de Datos

---

## 🏛️ Presentación Institucional
* **Universidad:** Universidad Nacional del Altiplano (UNA)
* **Facultad:** Facultad de Ingeniería Estadística e Informática
* **Unidad:** Escuela de Posgrado
* **Programa:** Doctorado en Ciencias de la Computación
* **Curso:** Estructuras de Datos Avanzadas / Algoritmos Avanzados

---

## 👥 Integrantes
* **Elmer Valerio Gómez Alcos** - [valesitoo@gmail.com](mailto:valesitoo@gmail.com)
* **David Fernández Chambilla** - [davi97.dfnc@gmail.com](mailto:davi97.dfnc@gmail.com)
* **Joel Dandy Tintaya Cahuapaza** - [tinta.jhoel20@gmail.com](mailto:tinta.jhoel20@gmail.com)
* **Luis Fernando Talizo Chambilla** - [luistalizo20@gmail.com](mailto:luistalizo20@gmail.com)

---

## 🚀 Descripción del Proyecto

Este repositorio contiene el desarrollo, código fuente y análisis experimental de un estudio comparativo de optimización de búsquedas sobre un dataset de transacciones masivas (~23 millones de registros, 1.26 GB). El objetivo principal es evaluar el rendimiento de estructuras de datos tradicionales frente a sus contrapartes potenciadas por Machine Learning (*Learned Indexes*), simulando el comportamiento de indexación física de motores de bases de datos relacionales como PostgreSQL.

### 📝 Resumen del Repositorio (About)
> Estudio experimental en C++ que compara tiempos de búsqueda entre estructuras de datos clásicas (Árbol B*, Skip List) y sus versiones con Machine Learning (Learned Indexes). Mide 10,000 consultas sobre un dataset inmutable de 23M de registros (1.26 GB), culminando en un artículo científico en LaTeX.

---

## 📊 Estructuras de Datos Comparadas

El núcleo del experimento evalúa y contrasta cuatro enfoques divididos en dos familias algorítmicas:

1. **Árbol B* (Tradicional):** Implementación clásica de la estructura de árbol balanceado optimizada para sistemas de almacenamiento, manteniendo una alta densidad de claves en los nodos.
2. **Árbol B* + Machine Learning:** Integración de modelos predictivos para estimar la posición de la clave dentro de las páginas del árbol, reduciendo los accesos a memoria/disco.
3. **Skip List (Tradicional):** Estructura probabilística basada en listas enlazadas por niveles, utilizando una distribución geométrica aleatoria para determinar la altura de los nodos.
4. **Skip List + Machine Learning:** Implementación avanzada de investigación donde las decisiones de salto y el crecimiento de los nodos son guiados por un modelo estadístico (Regresión Lineal/Logística) entrenado con la distribución de los datos, sustituyendo la aleatoriedad por predicciones matemáticas.

---

## 🛠️ Reglas de Oro y Metodología

* **Inmutabilidad del Dataset:** El archivo original `transactions_data.csv` (1.26 GB) **no se modifica, formatea ni parsea**. Se lee en crudo.
* **Índices Desacoplados:** Los archivos de índice generados se almacenan por separado y guardan la relación `(Key_ID, File_Offset)`.
* **Prueba de Estrés (Benchmarking):** Medición estricta del tiempo empleado para resolver un lote idéntico de 10,000 consultas de ID específicos.
* **Desarrollo Agilizado por IA:** El diseño y esqueleto del código fuente (en C++) se ha acelerado mediante ingeniería de prompts avanzada, enfocando el esfuerzo del investigador en la arquitectura del sistema, la calibración de modelos y la fase experimental.

---

## 📁 Contenido del Repositorio

El proyecto está organizado de la siguiente manera:

```text
├── src/                        # Código fuente del proyecto (C++)
│   ├── main.cpp                # Ejecutable principal y suite de pruebas (Benchmarking)
│   ├── indexer/                # Lógica de lectura de offsets y mapeo del CSV
│   ├── b_star_tree/            # Árbol B* tradicional y su versión optimizada con ML
│   └── skip_list/              # Skip List tradicional y optimizada con Regresión
│
├── data/                       # Carpeta contenedora del dataset e índices (Excluida en .gitignore)
│   └── README.md               # Instrucciones para descargar transactions_data.csv
│
├── literature/                 # Papers y literatura científica sobre Learned Indexes
│   └── README.md               # Lista de referencias bibliográficas de soporte
│
├── analysis/                   # Datos brutos de las mediciones y análisis
│   └── metrics_results.xlsx    # Archivo Excel con tiempos de búsqueda, varianza, medias y gráficos
│
├── paper/                      # Informe académico formal (Paper científico)
│   ├── main.tex                # Código fuente en LaTeX
│   ├── references.bib          # Bibliografía y referencias en formato BibTeX
│   └── report_final.pdf        # Artículo final compilado en formato de dos columnas
│
└── README.md                   # Este archivo con la descripción general e integrantes
```

---

## 📈 Resultados y Métricas (Resumen)
Los resultados detallados de los tiempos de ejecución para las 10,000 búsquedas se registrarán en `analysis/metrics_results.xlsx` y se discutirán en profundidad en el Paper.

| Estructura de Datos | Tiempo Total (ms) | Tiempo Promedio por Búsqueda (µs) | Desviación Estándar | Eficiencia vs. Tradicional |
| :--- | :--- | :--- | :--- | :--- |
| **Árbol B\*** (Tradicional) | *[Por definir]* | *[Por definir]* | *[Por definir]* | Baseline |
| **Árbol B\* + ML** | *[Por definir]* | *[Por definir]* | *[Por definir]* | *[Por definir %]* |
| **Skip List** (Tradicional) | *[Por definir]* | *[Por definir]* | *[Por definir]* | Baseline |
| **Skip List + ML** | *[Por definir]* | *[Por definir]* | *[Por definir]* | *[Por definir %]* |

---

## 📝 Entregables del Proyecto

Para la entrega y aprobación final del trabajo doctoral, el equipo deberá consolidar:
1. **Código Fuente Completo (C++17)**:
   * Implementación limpia, modular y optimizada de las 4 arquitecturas dentro de la carpeta `src/`.
   * Suite de pruebas automatizada en `src/main.cpp` capaz de medir y reportar tiempos en microsegundos.
2. **Archivos de Índices Desacoplados**:
   * Archivos externos `.idx` generados en disco que almacenen el mapa `(ID, offset)` sin modificar el archivo original de 1.26 GB.
3. **Análisis Estadístico Comparativo**:
   * Archivo Excel `analysis/metrics_results.xlsx` con los datos consolidados de las búsquedas (media, varianza, desviación estándar) y gráficos de barras/cajas.
4. **Artículo Científico en LaTeX**:
   * Artículo final redactado según el formato estándar IEEE en `paper/main.tex` y compilado como `paper/report_final.pdf`.

---

## 📅 Roadmap y Plan de Trabajo (Instrucciones para el Equipo)

Para avanzar ordenadamente en el desarrollo de la investigación, seguiremos las siguientes fases de implementación distribuyendo las tareas del equipo:

### 🔹 Fase 1: Extracción de Offsets y Serialización (`src/indexer/`)
* **Objetivo**: Mapear la ubicación en bytes (`offset`) de cada registro en el CSV inmutable basándonos en su columna `ID`.
* **Instrucciones**:
  * Descargar el dataset e inicializar `data/transactions_data.csv`.
  * Diseñar un cargador en C++ que recorra el archivo CSV secuencialmente una sola vez y extraiga el par `(Transaction_ID, Offset_Bytes)`.
  * Serializar este mapa en un archivo binario `.idx` rápido de cargar en memoria en subsecuentes ejecuciones para evitar re-parsear los 1.26 GB de datos.

### 🔹 Fase 2: Desarrollo de las Estructuras Tradicionales (Líneas Base)
* **Objetivo**: Implementar los algoritmos de búsqueda tradicionales.
* **Instrucciones**:
  * **Árbol B\* Tradicional (`src/b_star_tree/`)**: Diseñar la estructura jerárquica asegurando una densidad de nodos del $2/3$.
  * **Skip List Tradicional (`src/skip_list/`)**: Crear la lista probabilística estándar guiando la inserción y niveles con una distribución geométrica (azar).
  * Ambos índices deben poblarse usando el mapa de offsets previamente serializado.

### 🔹 Fase 3: Integración de Machine Learning (Learned Indexes)
* **Objetivo**: Reemplazar y optimizar los recorridos y saltos tradicionales con modelos de regresión entrenados.
* **Instrucciones**:
  * **Árbol B\* + ML**: Entrenar un modelo de predicción (p. ej., regresión lineal segmentada) sobre la función de distribución acumulada (CDF) de los IDs para predecir la página física de búsqueda con un error $\Delta$. Luego, realizar una búsqueda binaria local limitada.
  * **Skip List + ML**: Diseñar un estimador estadístico (regresión lineal o logística) que analice la densidad de claves locales para asignar predictivamente la altura de cada nodo de forma matemática en lugar de probabilística aleatoria.

### 🔹 Fase 4: Suite de Benchmarking e Informe Científico
* **Objetivo**: Ejecutar pruebas de estrés uniformes y documentar los hallazgos académicamente.
* **Instrucciones**:
  * En `src/main.cpp`, estructurar la suite de pruebas que lance **10,000 consultas aleatorias** idénticas sobre las 4 variantes de índice, midiendo la latencia exacta de cada una.
  * Exportar los tiempos, varianzas y desviación estándar a `analysis/metrics_results.xlsx`.
  * Redactar y compilar el paper académico en `paper/main.tex` utilizando LaTeX, detallando la metodología, comparando gráficamente los tiempos de latencia y discutiendo la relación entre costo de entrenamiento vs. velocidad de consulta.


---

## ⚙️ Requisitos y Ejecución
### Prerrequisitos
* Compilador de C++ (compatible con C++17 o superior, p. ej., `g++` o `clang`).
* Entorno para compilar LaTeX (como `pdflatex` o `TeX Live`) si deseas regenerar el PDF del informe.
* Python 3 (opcional, en caso de usar scripts auxiliares para el entrenamiento inicial de los modelos).

### Instrucciones Rápidas
1. Descarga el dataset [Financial Transactions Dataset: Analytics (Transactions Fraud Datasets)](https://www.kaggle.com/datasets/computingvictor/transactions-fraud-datasets/data?select=transactions_data.csv) de Kaggle y colócalo en la carpeta `data/` con el nombre `transactions_data.csv`.
2. Compila el código fuente:
   ```bash
   g++ -O3 -std=c++17 src/main.cpp -o benchmark_indexes
   ```
3. Ejecuta el benchmark para construir los índices y correr las 10,000 pruebas:
   ```bash
   ./benchmark_indexes
   ```

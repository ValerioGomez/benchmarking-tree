# Literatura de Soporte (Scientific Literature)

Esta carpeta está destinada a almacenar artículos científicos (papers), enlaces, notas de investigación y resúmenes de la literatura utilizada como base teórica para el benchmarking de *Learned Indexes* y estructuras tradicionales.

---

## 📚 Papers de Referencia Principales

### 1. The Seminal Paper: The Case for Learned Index Structures
* **Autores**: Tim Kraska, Alex Beutel, Ed H. Chi, Jeffrey Dean, Neoklis Polyzotis.
* **Publicación**: SIGMOD 2018.
* **Aporte clave**: Introduce el concepto de *Learned Indexes* proponiendo reemplazar las estructuras clásicas como los árboles B por modelos de Machine Learning (como CDFs aproximadas) para predecir la posición física de una clave.
* **Enlace/Referencia**: [https://doi.org/10.1145/3183713.3196909](https://doi.org/10.1145/3183713.3196909)

### 2. Learned Skip Lists
* **Autores/Investigaciones relevantes**: Diversos autores han extendido el concepto de listas de salto aprendidas.
* **Aporte clave**: Analiza cómo reemplazar la aleatoriedad probabilística (distribución geométrica) del crecimiento de los nodos y saltos en la Skip List clásica por predicciones basadas en regresión lineal o regresión logística sobre la distribución de claves, reduciendo el número de comparaciones necesarias en las búsquedas.
* **Estrategia**: Modelar la distribución acumulada (CDF) de las claves de búsqueda y asignar los saltos en base a la densidad predictiva del modelo.

### 3. FITing-Tree: A Data-aware Index Structure
* **Autores**: Alex Galakatos, Michael Markovitch, Carsten Binnig, Rodrigo Fonseca, Tim Kraska.
* **Publicación**: SIGMOD 2019.
* **Aporte clave**: Introduce el concepto de ajustar segmentos mediante regresión lineal con límites de error configurables en tiempo de búsqueda.

---

## 🔍 Criterios de Búsqueda y Fuentes de Datos (Scopus & WoS)

Para el desarrollo del marco teórico y el estado del arte de la investigación doctoral, el equipo utilizará las siguientes bases de datos indexadas de alto impacto:
* **Scopus** (Elsevier)
* **Web of Science (WoS)** (Clarivate)

### Ecuaciones de Búsqueda Sugeridas:
* `TITLE-ABS-KEY("Learned Index" OR "Learned Indexes" AND "B-Tree")`
* `TITLE-ABS-KEY("Learned Skip List" OR "Learned" AND "Skip List" AND "Regression")`
* `TITLE-ABS-KEY("index optimization" AND "machine learning" AND "database index")`

---

## 🗂️ Organización de la Carpeta

* **PDFs**: Coloque los artículos científicos clave en formato PDF en este directorio (`literature/`).
* **Citas**: Registre las referencias bibliográficas correspondientes en formato BibTeX en el archivo [references.bib](file:///c:/Users/vales/Documents/GitHub/benchmarking-tree/paper/references.bib) para la compilación en LaTeX.

# Carpeta de Datos (Data Directory)

Esta carpeta contiene el conjunto de datos original y los archivos de índices generados externamente.

> [!WARNING]
> Debido al tamaño del conjunto de datos (1.26 GB) y las políticas de Git, el archivo CSV de datos y los archivos `.idx` están excluidos del control de versiones mediante `.gitignore`.

---

## 📥 Obtención del Dataset Original

Para ejecutar las pruebas y construir los índices, siga los siguientes pasos:

1. **Descargar desde Kaggle**:
   * Dataset: **Financial Transactions Dataset: Analytics** (Transactions Fraud Datasets)
   * Enlace: [Kaggle Dataset Link](https://www.kaggle.com/datasets/computingvictor/transactions-fraud-datasets/data?select=transactions_data.csv)
2. **Nombre de Archivo**:
   * Guarde el archivo CSV descargado en esta carpeta (`data/`) bajo el nombre exacto de:
     `transactions_data.csv`

---

## 📊 Descripción del Dataset

* **Tamaño aproximado**: ~1.26 GB
* **Cantidad de registros**: ~23,000,000 de filas
* **Columna de Indexación (Clave)**: `ID` (Transaction ID)
* **Regla de Oro**: Este archivo permanece **estrictamente inmutable**. Ningún proceso debe alterar, pre-procesar ni guardar datos directamente en él.

---

## 📁 Archivos de Índice Generados (Salidas esperadas)

Cuando el programa `benchmark_indexes` se ejecute, creará en este directorio los siguientes archivos de índices desacoplados:

* `b_star_tree.idx`: Mapeo físico del Árbol B* tradicional.
* `b_star_tree_ml.idx`: Mapeo físico del Árbol B* optimizado con Machine Learning.
* `skip_list.idx`: Mapeo físico de la Skip List tradicional.
* `skip_list_ml.idx`: Mapeo físico de la Skip List optimizada con Machine Learning.

Cada entrada de estos índices asociará el `ID` de la transacción con el `offset` físico en bytes (puntero de archivo) dentro de `transactions_data.csv`.

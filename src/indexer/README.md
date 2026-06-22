# Mapeo e Indexación Física de Offsets (`src/indexer/`)

Este módulo se encarga de leer secuencialmente el archivo `data/transactions_data.csv` en crudo (sin cargarlo todo a memoria y de forma inmutable) para obtener las posiciones físicas (en bytes) de cada transacción.

---

## 🛠️ Funcionamiento y Diseño

1. **Lectura Inmutable**:
   * Se abre el archivo CSV mediante flujos de entrada en C++ (`std::ifstream`).
   * Para cada fila, se extrae el campo `ID` y se registra la posición del puntero de lectura (`file.tellg()`) al inicio de dicha fila en el archivo CSV.
2. **Estructura del Índice Desacoplado**:
   * Cada registro del índice almacena la tupla `(Key_ID, File_Offset)`.
   * `Key_ID`: Identificador de la transacción (tipo numérico o string).
   * `File_Offset`: Posición de inicio en bytes en el archivo CSV (tipo `std::streamoff` o `uint64_t`).
3. **Persistencia**:
   * Los índices se escriben en disco en formato binario para optimizar la velocidad de carga posterior, sin volver a recorrer el CSV completo de 1.26 GB en ejecuciones repetidas.

#pragma once

#include <vector>
#include <string>
#include <cstdint>

using namespace std;

// Estructura para almacenar el mapeo clave-offset
struct IndexEntry {
    uint64_t id;
    uint64_t offset;
};

class CSVIndexer {
public:
    /**
     * @brief Recorre secuencialmente el archivo CSV, extrae el ID y el offset en bytes,
     *        y guarda el resultado en un archivo de índice binario.
     * @param csvPath Ruta del archivo CSV original inmutable.
     * @param indexPath Ruta de destino para guardar el índice binario (.idx).
     * @return true si la indexación fue exitosa, false en caso contrario.
     */
    static bool generateIndex(const string& csvPath, const string& indexPath);

    /**
     * @brief Carga las entradas del índice binario a memoria de forma rápida.
     * @param indexPath Ruta del archivo de índice binario (.idx).
     * @return Vector con todas las entradas del índice.
     */
    static vector<IndexEntry> loadIndex(const string& indexPath);
};

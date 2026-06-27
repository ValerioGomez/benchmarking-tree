#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

struct IndexEntry {
    uint64_t id;
    uint64_t offset;
};

class CSVIndexer {
public:
    /**
     * @brief Recorre secuencialmente el archivo CSV, extrae el ID y el offset en bytes,
     *        y guarda el resultado en un archivo de índice binario.
     */
    static inline bool generateIndex(const string& csvPath, const string& indexPath) {
        ifstream file(csvPath, ios::binary);
        if (!file.is_open()) {
            cerr << "[CSVIndexer] Error: No se pudo abrir el archivo CSV en: " << csvPath << endl;
            return false;
        }

        cout << "[CSVIndexer] Iniciando indexacion de offsets del CSV..." << endl;

        string line;
        if (!getline(file, line)) {
            cerr << "[CSVIndexer] Error: El archivo CSV esta vacio." << endl;
            return false;
        }

        vector<IndexEntry> entries;
        entries.reserve(23500000);

        uint64_t count = 0;
        
        while (true) {
            uint64_t offset = file.tellg();
            if (!getline(file, line)) {
                break;
            }
            if (line.empty()) {
                continue;
            }

            uint64_t id = 0;
            size_t i = 0;
            bool valid = false;
            
            while (i < line.size() && line[i] >= '0' && line[i] <= '9') {
                id = id * 10 + (line[i] - '0');
                i++;
                valid = true;
            }

            if (valid && (i == line.size() || line[i] == ',')) {
                entries.push_back({id, offset});
            }

            count++;
            if (count % 5000000 == 0) {
                cout << "[CSVIndexer] Procesados " << (count / 1000000) << " millones de registros..." << endl;
            }
        }

        file.close();
        cout << "[CSVIndexer] Lectura del CSV finalizada. Total registros: " << entries.size() << endl;

        cout << "[CSVIndexer] Serializando indice en: " << indexPath << "..." << endl;
        ofstream outFile(indexPath, ios::binary);
        if (!outFile.is_open()) {
            cerr << "[CSVIndexer] Error: No se pudo crear el archivo de indice." << endl;
            return false;
        }

        size_t numEntries = entries.size();
        outFile.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));
        outFile.write(reinterpret_cast<const char*>(entries.data()), numEntries * sizeof(IndexEntry));
        outFile.close();

        cout << "[CSVIndexer] Indice generado. Registros: " << numEntries << endl;
        return true;
    }

    /**
     * @brief Carga las entradas del índice binario a memoria de forma rápida.
     */
    static inline vector<IndexEntry> loadIndex(const string& indexPath) {
        cout << "[CSVIndexer] Cargando indice binario desde: " << indexPath << "..." << endl;
        ifstream inFile(indexPath, ios::binary);
        if (!inFile.is_open()) {
            cerr << "[CSVIndexer] Advertencia: No se pudo abrir: " << indexPath << endl;
            return {};
        }

        size_t numEntries = 0;
        inFile.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));
        
        vector<IndexEntry> entries(numEntries);
        inFile.read(reinterpret_cast<char*>(entries.data()), numEntries * sizeof(IndexEntry));
        inFile.close();

        cout << "[CSVIndexer] Indice binario cargado. Total: " << numEntries << " entradas." << endl;
        return entries;
    }
};

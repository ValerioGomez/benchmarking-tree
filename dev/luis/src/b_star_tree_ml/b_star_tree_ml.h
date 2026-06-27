#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "../indexer/csv_indexer.h"

using namespace std;

/**
 * @brief Árbol B* optimizado mediante Machine Learning (Learned Index RMI).
 */
class BStarTreeML {
public:
    struct Model {
        double a;
        double b;
    };

private:
    double a_global, b_global; // Coeficientes globales para Skip List
    double a0, b0;             // Coeficientes Etapa 1
    int delta;                 // Error máximo absoluto acotado
    int M;                     // Número de submodelos en la Etapa 2
    int N;                     // Número total de elementos
    vector<Model> stage2_models;
    vector<IndexEntry> entries;

public:
    BStarTreeML() : a_global(0), b_global(0), a0(0), b0(0), delta(0), M(0), N(0) {}

    /**
     * @brief Carga los coeficientes entrenados y guarda la referencia de los datos indexados.
     * @param modelPath Ruta del archivo de coeficientes de texto.
     * @param sortedEntries Vector ordenado de IndexEntry.
     */
    bool loadModelsAndData(const string& modelPath, const vector<IndexEntry>& sortedEntries) {
        entries = sortedEntries;
        N = static_cast<int>(entries.size());

        ifstream file(modelPath);
        if (!file.is_open()) {
            cerr << "[BStarTreeML] Error: No se pudo abrir el archivo de coeficientes: " << modelPath << endl;
            return false;
        }

        // Leer coeficientes globales (Skip List)
        file >> a_global >> b_global;

        // Leer Etapa 1
        int delta_val = 0, M_val = 0, N_val = 0;
        file >> a0 >> b0 >> delta_val >> M_val >> N_val;
        delta = delta_val;
        M = M_val;

        // Leer Etapa 2
        stage2_models.resize(M);
        for (int i = 0; i < M; ++i) {
            file >> stage2_models[i].a >> stage2_models[i].b;
        }
        file.close();

        cout << "[BStarTreeML] RMI inicializado. Modelos Etapa 2: " << M 
             << " | Delta: " << delta << " | Registros: " << N << endl;
        return true;
    }

    /**
     * @brief Busca el ID utilizando la predicción del RMI de 2 etapas y búsqueda binaria local.
     */
    uint64_t search(uint64_t key) {
        if (N == 0) return 0;

        // Etapa 1: Predicción global
        double pred1 = a0 * static_cast<double>(key) + b0;
        int j = static_cast<int>(pred1 * M / N);
        if (j < 0) j = 0;
        if (j >= M) j = M - 1;

        // Etapa 2: Predicción local del submodelo
        double pred2 = stage2_models[j].a * static_cast<double>(key) + stage2_models[j].b;
        int pos = static_cast<int>(pred2);
        if (pos < 0) pos = 0;
        if (pos >= N) pos = N - 1;

        // Búsqueda binaria local en el intervalo de error acotado por delta
        int low = max(0, pos - delta);
        int high = min(N - 1, pos + delta);

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (entries[mid].id == key) {
                return entries[mid].offset;
            }
            if (entries[mid].id < key) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        return 0; // No encontrado
    }
};

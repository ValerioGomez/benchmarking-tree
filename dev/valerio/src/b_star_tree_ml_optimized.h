#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "../../../src/indexer/csv_indexer.h"

using namespace std;

/**
 * @class BStarTreeMLOptimized
 * @brief Árbol B* + ML con micro-optimizaciones a nivel de CPU.
 * 
 * Implementa dos optimizaciones clave:
 *   1. Eliminación de divisiones en caliente usando multiplicadores flotantes pre-calculados.
 *   2. Búsqueda local branchless para evitar predicciones fallidas de saltos en CPU.
 */
class BStarTreeMLOptimized {
public:
    struct Model {
        double a;
        double b;
    };

private:
    double a0, b0;
    double invN_M;  // Multiplicador pre-calculado para evitar divisiones (M / N)
    int delta;
    int M;
    int N;
    vector<Model> stage2_models;
    vector<IndexEntry> entries;

public:
    BStarTreeMLOptimized() : a0(0), b0(0), invN_M(0), delta(0), M(0), N(0) {}

    bool loadModelsAndData(const string& modelPath, const vector<IndexEntry>& sortedEntries) {
        entries = sortedEntries;
        N = static_cast<int>(entries.size());
        if (N == 0) return false;

        ifstream file(modelPath);
        if (!file.is_open()) {
            cerr << "[BStarTreeMLOpt] Error: No se pudo abrir: " << modelPath << endl;
            return false;
        }

        double dummy1, dummy2;
        file >> dummy1 >> dummy2; // Saltar coeficientes globales

        int delta_val = 0, M_val = 0, N_val = 0;
        file >> a0 >> b0 >> delta_val >> M_val >> N_val;
        delta = delta_val;
        M = M_val;

        invN_M = static_cast<double>(M) / static_cast<double>(N);

        stage2_models.resize(M);
        for (int i = 0; i < M; ++i) {
            file >> stage2_models[i].a >> stage2_models[i].b;
        }
        file.close();

        cout << "[BStarTreeMLOpt] RMI Optimizado cargado. Modelos: " << M 
             << " | Delta: " << delta << " | invN_M: " << invN_M << endl;
        return true;
    }

    /**
     * @brief Búsqueda optimizada con enrutamiento rápido y búsqueda local branchless.
     */
    uint64_t search(uint64_t key) const {
        if (N == 0) return 0;

        // Etapa 1: Predicción global (Multiplicación rápida en lugar de división)
        double pred1 = a0 * static_cast<double>(key) + b0;
        int j = static_cast<int>(pred1 * invN_M);
        
        // Clamping branchless usando min/max estándar (el compilador optimiza a cmov)
        j = max(0, min(M - 1, j));

        // Etapa 2: Predicción local del submodelo
        double pred2 = stage2_models[j].a * static_cast<double>(key) + stage2_models[j].b;
        int pos = static_cast<int>(pred2);
        pos = max(0, min(N - 1, pos));

        // Búsqueda binaria local acotada por delta
        int low = max(0, pos - delta);
        int high = min(N - 1, pos + delta);
        int count = high - low + 1;

        // Búsqueda binaria branchless (sin saltos condicionales en el bucle principal)
        while (count > 1) {
            int half = count / 2;
            int mid = low + half;
            // El compilador traduce esta asignación condicional a una instrucción CMOV (Conditional Move)
            low = (entries[mid].id <= key) ? mid : low;
            count -= half;
        }

        if (entries[low].id == key) {
            return entries[low].offset;
        }
        if (low + 1 < N && entries[low + 1].id == key) {
            return entries[low + 1].offset;
        }
        return 0;
    }
};

#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "../indexer/csv_indexer.h"

using namespace std;

/**
 * @brief Skip List optimizada por Machine Learning (Learned Skip List).
 *        Sustituye la probabilidad aleatoria por una función de altura guiada
 *        por la predicción del modelo de regresión de la CDF.
 */
class SkipListML {
public:
    struct Node {
        uint64_t key;
        uint64_t value;
        int height;
        int forwardHead;
    };

private:
    vector<Node> nodes;
    vector<int> forwardPool;
    int maxLevel;
    int headIndex;

    double a_global;
    double b_global;

    /**
     * @brief Determina la altura del nodo de forma matemática y determinista
     *        basándose en la posición predicha por el modelo de regresión.
     */
    int getLearnedHeight(uint64_t key) {
        double pred_idx = a_global * static_cast<double>(key) + b_global;
        int idx = static_cast<int>(round(pred_idx));
        if (idx < 0) idx = -idx;

        int height = 1;
        // Emular la distribución geométrica con factor de ramificación 4 (25%)
        // pero de forma determinista usando el índice predicho de la CDF
        while (height < maxLevel && (idx > 0) && (idx % 4 == 0)) {
            height++;
            idx /= 4;
        }
        return height;
    }

public:
    SkipListML(int maxLvl = 16) : maxLevel(maxLvl), headIndex(0), a_global(0), b_global(0) {}

    /**
     * @brief Carga los coeficientes globales de regresión para la Skip List.
     */
    bool loadModel(const string& modelPath) {
        ifstream file(modelPath);
        if (!file.is_open()) {
            cerr << "[SkipListML] Error: No se pudo abrir el archivo de coeficientes: " << modelPath << endl;
            return false;
        }
        file >> a_global >> b_global;
        file.close();
        cout << "[SkipListML] Modelo cargado. a_global = " << a_global 
             << " | b_global = " << b_global << endl;
        return true;
    }

    /**
     * @brief Busca una clave utilizando la Skip List balanceada por el modelo.
     */
    uint64_t search(uint64_t key) {
        if (nodes.empty()) return 0;

        int curr = headIndex;
        for (int l = maxLevel - 1; l >= 0; --l) {
            while (true) {
                int nextNodeIdx = forwardPool[nodes[curr].forwardHead + l];
                if (nextNodeIdx == -1) {
                    break;
                }
                if (nodes[nextNodeIdx].key < key) {
                    curr = nextNodeIdx;
                } else {
                    break;
                }
            }
        }

        int nextNodeIdx = forwardPool[nodes[curr].forwardHead + 0];
        if (nextNodeIdx != -1 && nodes[nextNodeIdx].key == key) {
            return nodes[nextNodeIdx].value;
        }
        return 0;
    }

    /**
     * @brief Construye la Skip List aprendida a partir de registros ordenados.
     */
    void build(const vector<IndexEntry>& entries) {
        if (entries.empty()) return;

        nodes.clear();
        forwardPool.clear();

        size_t N = entries.size();
        nodes.resize(N + 1);

        nodes[0].key = 0;
        nodes[0].value = 0;
        nodes[0].height = maxLevel;
        nodes[0].forwardHead = 0;

        int currentPoolSize = maxLevel;

        for (size_t i = 1; i <= N; ++i) {
            nodes[i].key = entries[i - 1].id;
            nodes[i].value = entries[i - 1].offset;
            nodes[i].height = getLearnedHeight(nodes[i].key);
            nodes[i].forwardHead = currentPoolSize;
            currentPoolSize += nodes[i].height;
        }

        forwardPool.assign(currentPoolSize, -1);

        vector<int> lastNodeAtLevel(maxLevel, 0);

        for (size_t i = 1; i <= N; ++i) {
            int h = nodes[i].height;
            for (int l = 0; l < h; ++l) {
                int prev = lastNodeAtLevel[l];
                forwardPool[nodes[prev].forwardHead + l] = static_cast<int>(i);
                lastNodeAtLevel[l] = static_cast<int>(i);
            }
        }
    }
};

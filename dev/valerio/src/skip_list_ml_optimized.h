#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "../../../src/indexer/csv_indexer.h"

using namespace std;

/**
 * @class SkipListMLOptimized
 * @brief Skip List + ML con enrutamiento por Landmarks predictivos (learned index).
 * 
 * Durante la construcción, crea un índice de landmarks espaciados. Al buscar,
 * realiza una búsqueda binaria rápida sobre los landmarks para encontrar el punto
 * de entrada más cercano de manera garantizada y correcta, reduciendo los saltos
 * de Skip List a un entorno local de máximo 1024 elementos.
 */
class SkipListMLOptimized {
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

    // Estructura de landmarks para enrutamiento rápido
    vector<int> landmarks;
    static const int LANDMARK_STEP = 1024;  // Un landmark cada 1024 nodos

    int getLearnedHeight(uint64_t key) {
        double pred_idx = a_global * static_cast<double>(key) + b_global;
        int idx = static_cast<int>(round(pred_idx));
        if (idx < 0) idx = -idx;

        int height = 1;
        while (height < maxLevel && (idx > 0) && (idx % 4 == 0)) {
            height++;
            idx /= 4;
        }
        return height;
    }

public:
    SkipListMLOptimized(int maxLvl = 16) : maxLevel(maxLvl), headIndex(0), a_global(0), b_global(0) {}

    bool loadModel(const string& modelPath) {
        ifstream file(modelPath);
        if (!file.is_open()) {
            cerr << "[SkipListMLOpt] Error: No se pudo abrir: " << modelPath << endl;
            return false;
        }
        file >> a_global >> b_global;
        file.close();
        cout << "[SkipListMLOpt] Modelo cargado. a_global = " << a_global << endl;
        return true;
    }

    /**
     * @brief Búsqueda optimizada por landmarks.
     */
    uint64_t search(uint64_t key) const {
        if (nodes.empty()) return 0;

        // Búsqueda binaria sobre los landmarks para encontrar el landmark más cercano que sea <= key
        int lo = 0, hi = static_cast<int>(landmarks.size()) - 1;
        int bucket = 0;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (nodes[landmarks[mid]].key <= key) {
                bucket = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }

        int curr = landmarks[bucket];

        // Hacemos el recorrido local desde el nivel 4 hacia abajo,
        // ya que el landmark está a una distancia menor a LANDMARK_STEP (1024 elementos)
        for (int l = 4; l >= 0; --l) {
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

        // Búsqueda final a nivel 0: verificamos curr y luego next
        if (nodes[curr].key == key) {
            return nodes[curr].value;
        }
        int nextNodeIdx = forwardPool[nodes[curr].forwardHead + 0];
        if (nextNodeIdx != -1 && nodes[nextNodeIdx].key == key) {
            return nodes[nextNodeIdx].value;
        }
        return 0;
    }

    void build(const vector<IndexEntry>& entries) {
        if (entries.empty()) return;

        nodes.clear();
        forwardPool.clear();
        landmarks.clear();

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
        headIndex = 0;

        vector<int> update(maxLevel, headIndex);

        for (size_t i = 1; i <= N; ++i) {
            int h = nodes[i].height;
            for (int l = 0; l < h; ++l) {
                int prev = update[l];
                forwardPool[nodes[prev].forwardHead + l] = i;
            }
            for (int l = 0; l < h; ++l) {
                update[l] = i;
            }
        }

        // Construir la tabla de landmarks acelerados
        int numLandmarks = (N + LANDMARK_STEP - 1) / LANDMARK_STEP;
        landmarks.reserve(numLandmarks + 1);
        
        // El primer landmark es el nodo head
        landmarks.push_back(headIndex);
        
        for (size_t i = 1; i <= N; i += LANDMARK_STEP) {
            landmarks.push_back(static_cast<int>(i));
        }

        cout << "[SkipListMLOpt] Skip List construida con " << landmarks.size() << " landmarks." << endl;
    }
};

#pragma once

#include <vector>
#include <iostream>
#include <random>
#include "../indexer/csv_indexer.h"

using namespace std;

/**
 * @brief Skip List Tradicional optimizada en memoria mediante un diseño plano (array-backed).
 */
class SkipListTraditional {
public:
    struct Node {
        uint64_t key;
        uint64_t value;
        int height;
        int forwardHead; // Índice de inicio en el pool global de punteros hacia adelante
    };

private:
    vector<Node> nodes;
    vector<int> forwardPool;
    int maxLevel;
    int headIndex;

    // Generación de nivel probabilístico clásico
    int randomHeight() {
        int height = 1;
        // p = 0.25 es estándar para optimizar espacio/tiempo (promedio de 1.33 punteros por nodo)
        while (height < maxLevel && (rand() % 100) < 25) {
            height++;
        }
        return height;
    }

public:
    SkipListTraditional(int maxLvl = 16) : maxLevel(maxLvl), headIndex(0) {
        // Inicializar generador pseudoaleatorio simple
        srand(42);
    }

    /**
     * @brief Busca un ID en la Skip List y retorna su offset físico.
     * @param key ID de transacción.
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

        // Descender al nivel 0 para verificar coincidencia
        int nextNodeIdx = forwardPool[nodes[curr].forwardHead + 0];
        if (nextNodeIdx != -1 && nodes[nextNodeIdx].key == key) {
            return nodes[nextNodeIdx].value;
        }
        return 0;
    }

    /**
     * @brief Construye la Skip List de forma ultra rápida a partir de registros ordenados.
     */
    void build(const vector<IndexEntry>& entries) {
        if (entries.empty()) return;

        nodes.clear();
        forwardPool.clear();

        size_t N = entries.size();
        nodes.resize(N + 1); // Indice 0 es la cabecera dummy

        // Configurar nodo cabecera
        nodes[0].key = 0;
        nodes[0].value = 0;
        nodes[0].height = maxLevel;
        nodes[0].forwardHead = 0;

        int currentPoolSize = maxLevel; // La cabecera ocupa maxLevel punteros

        // Asignar alturas y punteros base para cada nodo
        for (size_t i = 1; i <= N; ++i) {
            nodes[i].key = entries[i - 1].id;
            nodes[i].value = entries[i - 1].offset;
            nodes[i].height = randomHeight();
            nodes[i].forwardHead = currentPoolSize;
            currentPoolSize += nodes[i].height;
        }

        // Dimensionar el pool global de punteros adelante y llenarlo temporalmente con -1
        forwardPool.assign(currentPoolSize, -1);

        // Enlazar nodos secuencialmente
        vector<int> lastNodeAtLevel(maxLevel, 0); // Cabecera es el último nodo visto inicialmente

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

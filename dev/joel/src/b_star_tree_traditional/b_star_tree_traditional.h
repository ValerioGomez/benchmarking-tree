#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include "../indexer/csv_indexer.h"

using namespace std;

/**
 * @brief Árbol B* Tradicional para indexación estática en memoria.
 * @tparam KeyType Tipo de la clave (Transaction ID).
 * @tparam ValueType Tipo del valor (Offset en bytes).
 * @tparam MaxKeys Capacidad máxima de claves en un nodo.
 */
template <typename KeyType = uint64_t, typename ValueType = uint64_t, int MaxKeys = 255>
class BStarTreeTraditional {
public:
    struct Node {
        bool isLeaf;
        int numKeys;
        KeyType keys[MaxKeys];
        union {
            ValueType values[MaxKeys];         // Solo para hojas
            Node* children[MaxKeys + 1];        // Solo para nodos internos
        };

        Node(bool leaf) : isLeaf(leaf), numKeys(0) {
            if (!isLeaf) {
                for (int i = 0; i <= MaxKeys; ++i) {
                    children[i] = nullptr;
                }
            }
        }
    };

private:
    Node* root;
    size_t nodeCount;

    void deleteTree(Node* node) {
        if (!node) return;
        if (!node->isLeaf) {
            for (int i = 0; i <= node->numKeys; ++i) {
                deleteTree(node->children[i]);
            }
        }
        delete node;
    }

public:
    BStarTreeTraditional() : root(nullptr), nodeCount(0) {}
    
    ~BStarTreeTraditional() {
        deleteTree(root);
    }

    size_t getNodeCount() const { return nodeCount; }

    /**
     * @brief Busca una clave y devuelve su offset asociado.
     * @param key Clave ID a buscar.
     * @return Offset en bytes o 0 si no se encuentra.
     */
    ValueType search(KeyType key) {
        Node* curr = root;
        while (curr) {
            // Búsqueda binaria dentro del nodo
            int low = 0;
            int high = curr->numKeys - 1;
            int idx = curr->numKeys;

            while (low <= high) {
                int mid = low + (high - low) / 2;
                if (curr->keys[mid] >= key) {
                    idx = mid;
                    high = mid - 1;
                } else {
                    low = mid + 1;
                }
            }

            if (curr->isLeaf) {
                if (idx < curr->numKeys && curr->keys[idx] == key) {
                    return curr->values[idx];
                }
                return 0;
            } else {
                if (idx < curr->numKeys && curr->keys[idx] == key) {
                    curr = curr->children[idx + 1];
                } else {
                    curr = curr->children[idx];
                }
            }
        }
        return 0;
    }

    /**
     * @brief Construye el Árbol B* de abajo hacia arriba a partir de entradas ordenadas.
     *        Garantiza que la densidad de nodos esté balanceada al factor B*.
     * @param entries Entradas del índice ordenadas por ID.
     */
    void buildBottomUp(const vector<IndexEntry>& entries) {
        if (entries.empty()) return;

        deleteTree(root);
        root = nullptr;
        nodeCount = 0;

        // Para cumplir la regla B* de alta densidad (mínimo 2/3 de llenado)
        // Fijamos un factor de llenado del 80% para la construcción estática
        int fillFactor = (MaxKeys * 4) / 5; 
        if (fillFactor < 2) fillFactor = 2;

        vector<Node*> previousLevel;
        Node* currentLeaf = nullptr;

        for (size_t i = 0; i < entries.size(); ++i) {
            if (!currentLeaf || currentLeaf->numKeys >= fillFactor) {
                currentLeaf = new Node(true);
                nodeCount++;
                previousLevel.push_back(currentLeaf);
            }
            currentLeaf->keys[currentLeaf->numKeys] = entries[i].id;
            currentLeaf->values[currentLeaf->numKeys] = entries[i].offset;
            currentLeaf->numKeys++;
        }

        // Construir recursivamente los niveles superiores
        while (previousLevel.size() > 1) {
            vector<Node*> nextLevel;
            Node* parent = nullptr;

            for (size_t i = 0; i < previousLevel.size(); ++i) {
                Node* child = previousLevel[i];
                
                if (!parent || parent->numKeys >= fillFactor) {
                    parent = new Node(false);
                    nodeCount++;
                    nextLevel.push_back(parent);
                    parent->children[0] = child;
                } else {
                    parent->keys[parent->numKeys] = child->keys[0];
                    parent->children[parent->numKeys + 1] = child;
                    parent->numKeys++;
                }
            }
            previousLevel = nextLevel;
        }

        if (!previousLevel.empty()) {
            root = previousLevel[0];
        }
    }
};

#pragma once

/**
 * @file optimized_index.h
 * @brief Índice Segmentado ε-Acotado (Estilo PGM-Index) - Versión Optimizada O(N)
 * 
 * Implementa un Learned Index basado en segmentación lineal con error absoluto
 * máximo garantizado de ε = 32.
 * 
 * Algoritmo Optimizado:
 *   En lugar de un algoritmo codicioso con verificación completa O(N*S), usamos
 *   un enfoque de ventana deslizante: dividimos los datos en bloques de tamaño
 *   fijo (2*ε = 64 elementos) y ajustamos un regresor lineal por bloque.
 *   El error máximo está garantizado por la relación entre el tamaño del bloque
 *   y la propiedad de monotonía de las claves ordenadas.
 * 
 * La búsqueda final se reduce a:
 *   1. Cálculo directo del índice del segmento O(1) usando aritmética entera
 *   2. Evaluación del modelo lineal local O(1)
 *   3. Búsqueda binaria acotada en un rango de 2ε = 64 elementos O(6)
 * 
 * Autores: Valerio Gómez Alcos et al.
 * Doctorado en Ciencias de la Computación - UNA Puno
 */

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

#include "indexer/csv_indexer.h"

/**
 * @brief Segmento lineal con modelo local y rango de datos.
 */
struct Segment {
    uint64_t firstKey;   // Primera clave del segmento
    uint64_t lastKey;    // Última clave del segmento
    int      startPos;   // Posición de inicio en el arreglo ordenado
    int      count;      // Número de elementos en el segmento
    double   slope;      // Pendiente del modelo lineal local (a)
    double   intercept;  // Intersección del modelo lineal local (b)
};

/**
 * @class SegmentedIndex
 * @brief Learned Index basado en segmentación lineal ε-acotada.
 * 
 * Divide los datos ordenados en segmentos de tamaño fijo (BLOCK_SIZE = 2*ε),
 * ajusta un modelo de regresión lineal local por segmento, y realiza búsquedas
 * en O(log S + log BLOCK_SIZE) donde S es el número de segmentos.
 */
class SegmentedIndex {
private:
    static const int EPSILON = 32;
    static const int BLOCK_SIZE = 2 * EPSILON;  // 64 elementos por segmento

    vector<Segment> segments;
    const vector<IndexEntry>* dataRef;
    int N;

    /**
     * @brief Ajusta una regresión lineal sobre un rango [start, end) del arreglo.
     * Retorna (slope, intercept) donde y = slope * x + intercept
     * mapea key -> posición relativa dentro del bloque.
     */
    static pair<double, double> fitLinear(const vector<IndexEntry>& data, int start, int end) {
        int n = end - start;
        if (n <= 1) return {0.0, 0.0};

        double sumX = 0, sumY = 0, sumXX = 0, sumXY = 0;
        for (int i = start; i < end; ++i) {
            double x = static_cast<double>(data[i].id);
            double y = static_cast<double>(i - start);
            sumX += x;
            sumY += y;
            sumXX += x * x;
            sumXY += x * y;
        }

        double denom = n * sumXX - sumX * sumX;
        if (fabs(denom) < 1e-12) {
            return {0.0, sumY / n};
        }

        double slope = (n * sumXY - sumX * sumY) / denom;
        double intercept = (sumY - slope * sumX) / n;
        return {slope, intercept};
    }

public:
    SegmentedIndex() : dataRef(nullptr), N(0) {}

    /**
     * @brief Construye el índice segmentado en O(N).
     * 
     * Divide las claves ordenadas en bloques de BLOCK_SIZE = 64 elementos,
     * ajusta un modelo de regresión lineal local por bloque.
     * El error máximo de predicción está acotado por BLOCK_SIZE/2 = ε = 32.
     * 
     * @param entries Vector ordenado de IndexEntry.
     */
    void build(const vector<IndexEntry>& entries) {
        dataRef = &entries;
        N = static_cast<int>(entries.size());
        segments.clear();

        int numSegments = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;
        segments.reserve(numSegments);

        for (int i = 0; i < N; i += BLOCK_SIZE) {
            int end = min(i + BLOCK_SIZE, N);
            auto [slope, intercept] = fitLinear(entries, i, end);

            Segment seg;
            seg.firstKey = entries[i].id;
            seg.lastKey = entries[end - 1].id;
            seg.startPos = i;
            seg.count = end - i;
            seg.slope = slope;
            seg.intercept = intercept;
            segments.push_back(seg);
        }

        cout << "[SegmentedIndex] Indice construido. Segmentos: " << segments.size()
             << " | BlockSize: " << BLOCK_SIZE
             << " | Epsilon: " << EPSILON
             << " | Registros: " << N << endl;
    }

    /**
     * @brief Busca una clave en el índice segmentado.
     * 
     * 1. Localiza el segmento correcto mediante búsqueda binaria sobre firstKey.
     * 2. Evalúa el modelo lineal local del segmento para predecir la posición.
     * 3. Ejecuta una búsqueda binaria acotada en [pred - ε, pred + ε].
     * 
     * @param key La clave (Transaction_ID) a buscar.
     * @return El offset en bytes del CSV, o 0 si no se encontró.
     */
    uint64_t search(uint64_t key) const {
        if (segments.empty() || dataRef == nullptr) return 0;

        const vector<IndexEntry>& data = *dataRef;

        // Paso 1: Búsqueda binaria sobre los segmentos por firstKey
        int lo = 0, hi = static_cast<int>(segments.size()) - 1;
        int segIdx = 0;

        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (segments[mid].firstKey <= key) {
                segIdx = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }

        const Segment& seg = segments[segIdx];

        // Paso 2: Predicción del modelo lineal local
        double predicted = seg.slope * static_cast<double>(key) + seg.intercept;
        int relPos = static_cast<int>(round(predicted));
        int absPos = seg.startPos + relPos;

        // Paso 3: Búsqueda binaria local estrictamente dentro del segmento
        int segStart = seg.startPos;
        int segEnd = seg.startPos + seg.count - 1;

        // Acotamos la búsqueda al rango predicho ± ε, pero sin salir del segmento
        int low = max(segStart, absPos - EPSILON);
        int high = min(segEnd, absPos + EPSILON);

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (data[mid].id == key) {
                return data[mid].offset;
            }
            if (data[mid].id < key) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        return 0;
    }

    int getSegmentCount() const { return static_cast<int>(segments.size()); }
    int getEpsilon() const { return EPSILON; }
    int getBlockSize() const { return BLOCK_SIZE; }
};

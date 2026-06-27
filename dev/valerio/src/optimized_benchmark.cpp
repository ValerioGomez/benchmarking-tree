/**
 * @file optimized_benchmark.cpp
 * @brief Suite de Benchmarking Optimizada: Índice Segmentado ε-Acotado + Paralelismo Multihilo
 * 
 * Este ejecutable es independiente del benchmark base (src/main.cpp) y opera
 * sin modificar ningún archivo del código original.
 * 
 * Funcionalidades:
 *   1. Carga el índice binario existente (data/offsets.idx) sin modificaciones.
 *   2. Construye el nuevo índice segmentado ε-acotado (Optimización A).
 *   3. Compara su rendimiento secuencial contra el RMI base (Optimización existente).
 *   4. Ejecuta las 10,000 consultas en paralelo usando std::thread (Optimización B).
 *   5. Reporta speedup, QPS y exporta latencias para pruebas estadísticas.
 * 
 * Autores: Valerio Gómez Alcos et al.
 * Doctorado en Ciencias de la Computación - UNA Puno
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <mutex>
#include <numeric>
#include <iomanip>

using namespace std;

// Incluir las estructuras de datos del proyecto base
#include "../../../src/indexer/csv_indexer.h"
#include "../../../src/b_star_tree_traditional/b_star_tree_traditional.h"
#include "../../../src/b_star_tree_ml/b_star_tree_ml.h"
#include "../../../src/skip_list_traditional/skip_list_traditional.h"
#include "../../../src/skip_list_ml/skip_list_ml.h"

// Incluir el índice segmentado optimizado
#include "optimized_index.h"

// ============================================================
// ESTRUCTURA AUXILIAR PARA ESTADÍSTICAS
// ============================================================
struct BenchStats {
    double buildTimeMs;
    double avgLatencyUs;
    double stddevUs;
    double totalTimeMs;
};

BenchStats computeStats(double buildMs, const vector<double>& latencies) {
    BenchStats s;
    s.buildTimeMs = buildMs;
    double sum = 0;
    for (double v : latencies) sum += v;
    s.avgLatencyUs = sum / latencies.size();
    s.totalTimeMs = sum / 1000.0;
    double sq = 0;
    for (double v : latencies) sq += (v - s.avgLatencyUs) * (v - s.avgLatencyUs);
    s.stddevUs = sqrt(sq / latencies.size());
    return s;
}

void printStats(const string& name, const BenchStats& s) {
    cout << "  " << left << setw(35) << name 
         << "| Promedio: " << fixed << setprecision(5) << s.avgLatencyUs << " us"
         << " | Desv.Est: " << s.stddevUs << " us"
         << " | Construc: " << setprecision(2) << s.buildTimeMs << " ms" << endl;
}

void exportResultsJSON(const string& path, 
                       const BenchStats& s_bt, 
                       const BenchStats& s_bm, 
                       const BenchStats& s_st, 
                       const BenchStats& s_sm, 
                       const BenchStats& s_seg) {
    ofstream file(path);
    if (file.is_open()) {
        file << "{\n"
             << "  \"b_star_traditional\": {\n"
             << "    \"build_time_ms\": " << s_bt.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_bt.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_bt.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_bt.stddevUs * s_bt.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_bt.stddevUs << "\n"
             << "  },\n"
             << "  \"b_star_ml\": {\n"
             << "    \"build_time_ms\": " << s_bm.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_bm.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_bm.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_bm.stddevUs * s_bm.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_bm.stddevUs << "\n"
             << "  },\n"
             << "  \"skip_list_traditional\": {\n"
             << "    \"build_time_ms\": " << s_st.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_st.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_st.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_st.stddevUs * s_st.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_st.stddevUs << "\n"
             << "  },\n"
             << "  \"skip_list_ml\": {\n"
             << "    \"build_time_ms\": " << s_sm.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_sm.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_sm.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_sm.stddevUs * s_sm.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_sm.stddevUs << "\n"
             << "  },\n"
             << "  \"segmented_index\": {\n"
             << "    \"build_time_ms\": " << s_seg.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_seg.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_seg.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_seg.stddevUs * s_seg.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_seg.stddevUs << "\n"
             << "  }\n"
             << "}\n";
        file.close();
        cout << "[Opt] Resultados consolidados exportados a JSON: " << path << endl;
    }
}

// ============================================================
// FUNCIÓN DE BENCHMARK SECUENCIAL (Para una estructura)
// ============================================================
template<typename SearchFn>
vector<double> benchmarkSequential(const vector<uint64_t>& keys, SearchFn fn) {
    vector<double> latencies;
    latencies.reserve(keys.size());
    for (uint64_t key : keys) {
        auto t1 = chrono::high_resolution_clock::now();
        volatile uint64_t result = fn(key);  // volatile previene optimización
        auto t2 = chrono::high_resolution_clock::now();
        chrono::duration<double, micro> diff = t2 - t1;
        latencies.push_back(diff.count());
        (void)result;
    }
    return latencies;
}

// ============================================================
// FUNCIÓN DE BENCHMARK PARALELO (Optimización B)
// ============================================================
template<typename SearchFn>
double benchmarkParallel(const vector<uint64_t>& keys, SearchFn fn, int numThreads) {
    // Dividir las consultas entre los hilos
    vector<thread> threads;
    int chunkSize = static_cast<int>(keys.size()) / numThreads;
    int remainder = static_cast<int>(keys.size()) % numThreads;

    auto start = chrono::high_resolution_clock::now();

    int offset = 0;
    for (int t = 0; t < numThreads; ++t) {
        int thisChunk = chunkSize + (t < remainder ? 1 : 0);
        int startIdx = offset;
        int endIdx = offset + thisChunk;
        offset = endIdx;

        threads.emplace_back([&keys, &fn, startIdx, endIdx]() {
            for (int i = startIdx; i < endIdx; ++i) {
                volatile uint64_t result = fn(keys[i]);
                (void)result;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = end - start;
    return elapsed.count();
}

// ============================================================
// MAIN
// ============================================================
int main() {
    cout << "===========================================================" << endl;
    cout << "  SUITE DE OPTIMIZACION: INDICE SEGMENTADO + PARALELISMO" << endl;
    cout << "  Doctorado en Ciencias de la Computacion - UNA" << endl;
    cout << "===========================================================" << endl;

    string indexPath = "data/offsets.idx";
    string modelPath = "data/model_coefficients.txt";
    string csvPath = "data/transactions_data.csv";

    // ----------------------------------------------------------------
    // FASE 1: Cargar el índice binario
    // ----------------------------------------------------------------
    auto loadStart = chrono::high_resolution_clock::now();
    vector<IndexEntry> entries = CSVIndexer::loadIndex(indexPath);
    auto loadEnd = chrono::high_resolution_clock::now();
    if (entries.empty()) {
        cerr << "[Opt] Error: El indice esta vacio o no existe." << endl;
        return 1;
    }
    double loadMs = chrono::duration<double, milli>(loadEnd - loadStart).count();

    sort(entries.begin(), entries.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return a.id < b.id;
    });
    cout << "[Opt] Indice cargado y ordenado en: " << loadMs << " ms." << endl;

    // ----------------------------------------------------------------
    // FASE 2: Generar 10,000 consultas aleatorias
    // ----------------------------------------------------------------
    mt19937_64 rng(42);  // Semilla fija para reproducibilidad
    uniform_int_distribution<size_t> dist(0, entries.size() - 1);
    vector<uint64_t> queryKeys(10000);
    for (auto& key : queryKeys) {
        key = entries[dist(rng)].id;
    }
    cout << "[Opt] 10,000 consultas generadas." << endl;

    // ----------------------------------------------------------------
    // FASE 3: Construcción de las estructuras
    // ----------------------------------------------------------------
    cout << "\n============ CONSTRUCCION DE ESTRUCTURAS ============\n" << endl;

    // 3.1 Árbol B* Tradicional
    BStarTreeTraditional<uint64_t, uint64_t, 255> bStarTrad;
    auto bt1 = chrono::high_resolution_clock::now();
    bStarTrad.buildBottomUp(entries);
    auto bt2 = chrono::high_resolution_clock::now();
    double bStarTradBuildMs = chrono::duration<double, milli>(bt2 - bt1).count();

    // 3.2 Árbol B* + ML (RMI existente)
    BStarTreeML bStarML;
    auto bm1 = chrono::high_resolution_clock::now();
    bStarML.loadModelsAndData(modelPath, entries);
    auto bm2 = chrono::high_resolution_clock::now();
    double bStarMLBuildMs = chrono::duration<double, milli>(bm2 - bm1).count();

    // 3.3 Skip List Tradicional
    SkipListTraditional skipTrad(16);
    auto st1 = chrono::high_resolution_clock::now();
    skipTrad.build(entries);
    auto st2 = chrono::high_resolution_clock::now();
    double skipTradBuildMs = chrono::duration<double, milli>(st2 - st1).count();

    // 3.4 Skip List + ML
    SkipListML skipML(16);
    auto sm1 = chrono::high_resolution_clock::now();
    skipML.loadModel(modelPath);
    skipML.build(entries);
    auto sm2 = chrono::high_resolution_clock::now();
    double skipMLBuildMs = chrono::duration<double, milli>(sm2 - sm1).count();

    // 3.5 ★ NUEVO: Índice Segmentado ε-Acotado (Optimización A)
    SegmentedIndex segIndex;
    auto sg1 = chrono::high_resolution_clock::now();
    segIndex.build(entries);
    auto sg2 = chrono::high_resolution_clock::now();
    double segBuildMs = chrono::duration<double, milli>(sg2 - sg1).count();

    // ----------------------------------------------------------------
    // FASE 4: Benchmark Secuencial (10,000 consultas en un hilo)
    // ----------------------------------------------------------------
    cout << "\n============ BENCHMARK SECUENCIAL (1 Hilo) ============\n" << endl;

    auto latBStarTrad = benchmarkSequential(queryKeys, [&](uint64_t k) { return bStarTrad.search(k); });
    auto statsBStarTrad = computeStats(bStarTradBuildMs, latBStarTrad);
    printStats("Arbol B* Tradicional", statsBStarTrad);

    auto latBStarML = benchmarkSequential(queryKeys, [&](uint64_t k) { return bStarML.search(k); });
    auto statsBStarML = computeStats(bStarMLBuildMs, latBStarML);
    printStats("Arbol B* + ML (RMI)", statsBStarML);

    auto latSkipTrad = benchmarkSequential(queryKeys, [&](uint64_t k) { return skipTrad.search(k); });
    auto statsSkipTrad = computeStats(skipTradBuildMs, latSkipTrad);
    printStats("Skip List Tradicional", statsSkipTrad);

    auto latSkipML = benchmarkSequential(queryKeys, [&](uint64_t k) { return skipML.search(k); });
    auto statsSkipML = computeStats(skipMLBuildMs, latSkipML);
    printStats("Skip List + ML", statsSkipML);

    auto latSegIdx = benchmarkSequential(queryKeys, [&](uint64_t k) { return segIndex.search(k); });
    auto statsSegIdx = computeStats(segBuildMs, latSegIdx);
    printStats("* Indice Segmentado e-Acotado", statsSegIdx);

    string resultsJSONPath = "dev/valerio/analysis/benchmark_results_optimized.json";
    exportResultsJSON(resultsJSONPath, statsBStarTrad, statsBStarML, statsSkipTrad, statsSkipML, statsSegIdx);

    // ----------------------------------------------------------------
    // FASE 5: Verificación de Correctitud
    // ----------------------------------------------------------------
    cout << "\n============ VERIFICACION DE CORRECTITUD ============\n" << endl;
    bool allCorrect = true;
    int verified = 0;
    for (size_t i = 0; i < queryKeys.size(); ++i) {
        uint64_t key = queryKeys[i];
        // Ground truth: búsqueda binaria directa sobre el arreglo ordenado
        auto it = lower_bound(entries.begin(), entries.end(), key,
            [](const IndexEntry& e, uint64_t k) { return e.id < k; });
        uint64_t ref = (it != entries.end() && it->id == key) ? it->offset : 0;
        uint64_t opt = segIndex.search(key);
        if (ref != opt) {
            cerr << "[ERROR] Discrepancia en clave " << key
                 << ": ref=" << ref << " opt=" << opt << endl;
            allCorrect = false;
            break;
        }
        verified++;
    }
    if (allCorrect) {
        cout << "[Opt] VERIFICACION EXITOSA: " << verified << " consultas verificadas."
             << " El indice segmentado retorna offsets 100% correctos." << endl;
    }

    // ----------------------------------------------------------------
    // FASE 6: Benchmark Paralelo Multihilo (Optimización B)
    // ----------------------------------------------------------------
    int numCores = static_cast<int>(thread::hardware_concurrency());
    if (numCores == 0) numCores = 4;

    cout << "\n============ BENCHMARK PARALELO (" << numCores << " Hilos) ============\n" << endl;

    // Medir tiempo secuencial del índice segmentado (para comparar speedup)
    double seqTimeMs = benchmarkParallel(queryKeys, [&](uint64_t k) { return segIndex.search(k); }, 1);
    cout << "  Secuencial (1 hilo):  " << fixed << setprecision(4) << seqTimeMs << " ms" << endl;

    // Paralelismo con 2, 4, 8 hilos y todos los núcleos
    vector<int> threadCounts = {2, 4};
    if (numCores >= 8) threadCounts.push_back(8);
    if (numCores > 8) threadCounts.push_back(numCores);

    vector<pair<int, double>> parallelResults;
    parallelResults.push_back({1, seqTimeMs});

    for (int nT : threadCounts) {
        double parTimeMs = benchmarkParallel(queryKeys, [&](uint64_t k) { return segIndex.search(k); }, nT);
        double speedup = seqTimeMs / parTimeMs;
        cout << "  Paralelo (" << nT << " hilos):  " << parTimeMs << " ms"
             << " | Speedup: " << setprecision(2) << speedup << "x"
             << " | QPS: " << static_cast<int>(10000.0 / (parTimeMs / 1000.0)) << endl;
        parallelResults.push_back({nT, parTimeMs});
    }

    // ----------------------------------------------------------------
    // FASE 7: Exportar Latencias para Pruebas Estadísticas
    // ----------------------------------------------------------------
    string latPath = "dev/valerio/analysis/latencies_optimized.csv";
    ofstream latFile(latPath);
    if (latFile.is_open()) {
        latFile << "key,b_star_traditional,b_star_ml,skip_list_traditional,skip_list_ml,segmented_index\n";
        for (size_t i = 0; i < queryKeys.size(); ++i) {
            latFile << queryKeys[i] << ","
                    << latBStarTrad[i] << ","
                    << latBStarML[i] << ","
                    << latSkipTrad[i] << ","
                    << latSkipML[i] << ","
                    << latSegIdx[i] << "\n";
        }
        latFile.close();
        cout << "\n[Opt] Latencias exportadas a: " << latPath << endl;
    }

    // Exportar resultados del paralelismo
    string parPath = "dev/valerio/analysis/parallel_results.csv";
    ofstream parFile(parPath);
    if (parFile.is_open()) {
        parFile << "threads,time_ms,speedup,qps\n";
        for (auto& [nT, tMs] : parallelResults) {
            double sp = seqTimeMs / tMs;
            int qps = static_cast<int>(10000.0 / (tMs / 1000.0));
            parFile << nT << "," << tMs << "," << sp << "," << qps << "\n";
        }
        parFile.close();
        cout << "[Opt] Resultados paralelos exportados a: " << parPath << endl;
    }

    // ----------------------------------------------------------------
    // RESUMEN COMPARATIVO FINAL
    // ----------------------------------------------------------------
    cout << "\n============ RESUMEN COMPARATIVO FINAL ============\n" << endl;
    cout << "  Estructura                         | Latencia Prom.  | Ganancia vs B* Trad." << endl;
    cout << "  -----------------------------------|-----------------|---------------------" << endl;
    
    double baseline = statsBStarTrad.avgLatencyUs;
    auto pctGain = [&](double lat) -> double { return (1.0 - lat / baseline) * 100.0; };
    
    cout << "  " << left << setw(37) << "Arbol B* Tradicional" 
         << "| " << setw(15) << statsBStarTrad.avgLatencyUs << " us | Baseline" << endl;
    cout << "  " << setw(37) << "Arbol B* + ML (RMI)"
         << "| " << setw(15) << statsBStarML.avgLatencyUs << " us | +" << fixed << setprecision(1) << pctGain(statsBStarML.avgLatencyUs) << " %" << endl;
    cout << "  " << setw(37) << "Skip List Tradicional"
         << "| " << setw(15) << statsSkipTrad.avgLatencyUs << " us | " << pctGain(statsSkipTrad.avgLatencyUs) << " %" << endl;
    cout << "  " << setw(37) << "Skip List + ML"
         << "| " << setw(15) << statsSkipML.avgLatencyUs << " us | " << pctGain(statsSkipML.avgLatencyUs) << " %" << endl;
    cout << "  " << setw(37) << "* INDICE SEGMENTADO e-ACOTADO"
         << "| " << setw(15) << statsSegIdx.avgLatencyUs << " us | +" << pctGain(statsSegIdx.avgLatencyUs) << " %" << endl;

    // Modo interactivo
    cout << "\n===========================================================" << endl;
    cout << "  CONSULTAS INTERACTIVAS" << endl;
    cout << "===========================================================" << endl;
    cout << "[Interactive] Desea buscar IDs? (s/n): ";
    char choice;
    if (cin >> choice && (choice == 's' || choice == 'S')) {
        while (true) {
            cout << "\n[Interactive] Ingrese el ID (o 0 para salir): ";
            uint64_t targetId;
            if (!(cin >> targetId) || targetId == 0) break;

            auto t1 = chrono::high_resolution_clock::now();
            uint64_t o1 = bStarTrad.search(targetId);
            auto t2 = chrono::high_resolution_clock::now();
            uint64_t o2 = bStarML.search(targetId);
            auto t3 = chrono::high_resolution_clock::now();
            uint64_t o3 = skipTrad.search(targetId);
            auto t4 = chrono::high_resolution_clock::now();
            uint64_t o4 = skipML.search(targetId);
            auto t5 = chrono::high_resolution_clock::now();
            uint64_t o5 = segIndex.search(targetId);
            auto t6 = chrono::high_resolution_clock::now();

            cout << "\n----------------------------------------------------------" << endl;
            cout << "  RESULTADOS PARA ID: " << targetId << endl;
            cout << "----------------------------------------------------------" << endl;
            cout << "1. B* Tradicional:          Offset=" << o1 << " | " << chrono::duration<double, nano>(t2-t1).count() << " ns" << endl;
            cout << "2. B* + ML (RMI):           Offset=" << o2 << " | " << chrono::duration<double, nano>(t3-t2).count() << " ns" << endl;
            cout << "3. Skip List Tradicional:   Offset=" << o3 << " | " << chrono::duration<double, nano>(t4-t3).count() << " ns" << endl;
            cout << "4. Skip List + ML:          Offset=" << o4 << " | " << chrono::duration<double, nano>(t5-t4).count() << " ns" << endl;
            cout << "5. *Segmentado e-Acotado:   Offset=" << o5 << " | " << chrono::duration<double, nano>(t6-t5).count() << " ns" << endl;
            cout << "----------------------------------------------------------" << endl;

            if (o1 != 0) {
                ifstream csvFile(csvPath, ios::binary);
                if (csvFile.is_open()) {
                    csvFile.seekg(o1);
                    string line;
                    if (getline(csvFile, line)) {
                        cout << "[CSV] " << line << endl;
                    }
                    csvFile.close();
                }
            } else {
                cout << "[CSV] ID no encontrado." << endl;
            }
        }
    }

    return 0;
}

/**
 * @file optimized_benchmark.cpp
 * @brief Suite de Benchmarking de Optimización de Modelos ML ("Antes vs. Después")
 * 
 * Evalúa y compara 6 estructuras de datos indexados:
 *   1. Árbol B* Tradicional
 *   2. Árbol B* + ML (RMI Baseline - Antes)
 *   3. Árbol B* + ML (RMI Optimizado - Después)
 *   4. Skip List Tradicional
 *   5. Skip List + ML (Baseline - Antes)
 *   6. Skip List + ML (Optimizado - Después)
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

// Incluir cabeceras del proyecto base (usando rutas relativas al archivo)
#include "../../../src/indexer/csv_indexer.h"
#include "../../../src/b_star_tree_traditional/b_star_tree_traditional.h"
#include "../../../src/b_star_tree_ml/b_star_tree_ml.h"
#include "../../../src/skip_list_traditional/skip_list_traditional.h"
#include "../../../src/skip_list_ml/skip_list_ml.h"

// Incluir cabeceras optimizadas
#include "b_star_tree_ml_optimized.h"
#include "skip_list_ml_optimized.h"

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
                       const BenchStats& s_bmo, 
                       const BenchStats& s_st, 
                       const BenchStats& s_sm, 
                       const BenchStats& s_smo) {
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
             << "  \"b_star_ml_optimized\": {\n"
             << "    \"build_time_ms\": " << s_bmo.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_bmo.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_bmo.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_bmo.stddevUs * s_bmo.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_bmo.stddevUs << "\n"
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
             << "  \"skip_list_ml_optimized\": {\n"
             << "    \"build_time_ms\": " << s_smo.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s_smo.totalTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s_smo.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << (s_smo.stddevUs * s_smo.stddevUs) << ",\n"
             << "    \"stddev_us\": " << s_smo.stddevUs << "\n"
             << "  }\n"
             << "}\n";
        file.close();
        cout << "[Opt] Resultados exportados a JSON." << endl;
    }
}

// ============================================================
// FUNCIONES DE BENCHMARK
// ============================================================
template<typename SearchFn>
vector<double> benchmarkSequential(const vector<uint64_t>& keys, SearchFn fn) {
    vector<double> latencies;
    latencies.reserve(keys.size());
    for (uint64_t key : keys) {
        auto t1 = chrono::high_resolution_clock::now();
        volatile uint64_t result = fn(key);
        auto t2 = chrono::high_resolution_clock::now();
        chrono::duration<double, micro> diff = t2 - t1;
        latencies.push_back(diff.count());
        (void)result;
    }
    return latencies;
}

template<typename SearchFn>
double benchmarkParallel(const vector<uint64_t>& keys, SearchFn fn, int numThreads) {
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
                volatile uint64_t r = fn(keys[i]);
                (void)r;
            }
        });
    }
    for (auto& t : threads) t.join();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(end - start).count();
}

int main() {
    cout << "===========================================================" << endl;
    cout << "  SUITE DE OPTIMIZACION DE MODELOS: ANTES VS DESPUES" << endl;
    cout << "  Doctorado en Ciencias de la Computacion - UNA" << endl;
    cout << "===========================================================" << endl;

    string indexPath = "data/offsets.idx";
    string modelPath = "data/model_coefficients.txt";
    string csvPath = "data/transactions_data.csv";

    // 1. Cargar el índice binario
    auto loadStart = chrono::high_resolution_clock::now();
    vector<IndexEntry> entries = CSVIndexer::loadIndex(indexPath);
    auto loadEnd = chrono::high_resolution_clock::now();
    if (entries.empty()) {
        cerr << "[Opt] Error: El indice esta vacio." << endl;
        return 1;
    }
    double loadMs = chrono::duration<double, milli>(loadEnd - loadStart).count();
    
    sort(entries.begin(), entries.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return a.id < b.id;
    });
    cout << "[Opt] Indice cargado y ordenado en: " << loadMs << " ms." << endl;

    // 2. Generar consultas
    mt19937_64 rng(42);
    uniform_int_distribution<size_t> dist(0, entries.size() - 1);
    vector<uint64_t> queryKeys(10000);
    for (auto& key : queryKeys) {
        key = entries[dist(rng)].id;
    }

    // 3. Construir estructuras
    cout << "\n============ CONSTRUCCION DE ESTRUCTURAS ============\n" << endl;

    // B* Tradicional
    BStarTreeTraditional<uint64_t, uint64_t, 255> bStarTrad;
    auto bt1 = chrono::high_resolution_clock::now();
    bStarTrad.buildBottomUp(entries);
    auto bt2 = chrono::high_resolution_clock::now();
    double bStarTradBuildMs = chrono::duration<double, milli>(bt2 - bt1).count();

    // B* ML Baseline (Antes)
    BStarTreeML bStarML;
    auto bm1 = chrono::high_resolution_clock::now();
    bStarML.loadModelsAndData(modelPath, entries);
    auto bm2 = chrono::high_resolution_clock::now();
    double bStarMLBuildMs = chrono::duration<double, milli>(bm2 - bm1).count();

    // B* ML Optimizado (Después - Radix/Division-free + Branchless)
    BStarTreeMLOptimized bStarMLOpt;
    auto bmo1 = chrono::high_resolution_clock::now();
    bStarMLOpt.loadModelsAndData(modelPath, entries);
    auto bmo2 = chrono::high_resolution_clock::now();
    double bStarMLOptBuildMs = chrono::duration<double, milli>(bmo2 - bmo1).count();

    // Skip List Tradicional
    SkipListTraditional skipTrad(16);
    auto st1 = chrono::high_resolution_clock::now();
    skipTrad.build(entries);
    auto st2 = chrono::high_resolution_clock::now();
    double skipTradBuildMs = chrono::duration<double, milli>(st2 - st1).count();

    // Skip List ML Baseline (Antes)
    SkipListML skipML(16);
    auto sm1 = chrono::high_resolution_clock::now();
    skipML.loadModel(modelPath);
    skipML.build(entries);
    auto sm2 = chrono::high_resolution_clock::now();
    double skipMLBuildMs = chrono::duration<double, milli>(sm2 - sm1).count();

    // Skip List ML Optimizada (Después - Landmark enrutamiento)
    SkipListMLOptimized skipMLOpt(16);
    auto smo1 = chrono::high_resolution_clock::now();
    skipMLOpt.loadModel(modelPath);
    skipMLOpt.build(entries);
    auto smo2 = chrono::high_resolution_clock::now();
    double skipMLOptBuildMs = chrono::duration<double, milli>(smo2 - smo1).count();

    // 4. Benchmark Secuencial
    cout << "\n============ BENCHMARK SECUENCIAL (1 Hilo) ============\n" << endl;

    auto latBStarTrad = benchmarkSequential(queryKeys, [&](uint64_t k) { return bStarTrad.search(k); });
    auto statsBStarTrad = computeStats(bStarTradBuildMs, latBStarTrad);
    printStats("Arbol B* Tradicional", statsBStarTrad);

    auto latBStarML = benchmarkSequential(queryKeys, [&](uint64_t k) { return bStarML.search(k); });
    auto statsBStarML = computeStats(bStarMLBuildMs, latBStarML);
    printStats("Arbol B* + ML (RMI Base)", statsBStarML);

    auto latBStarMLOpt = benchmarkSequential(queryKeys, [&](uint64_t k) { return bStarMLOpt.search(k); });
    auto statsBStarMLOpt = computeStats(bStarMLOptBuildMs, latBStarMLOpt);
    printStats("Arbol B* + ML (RMI Optimizado)", statsBStarMLOpt);

    auto latSkipTrad = benchmarkSequential(queryKeys, [&](uint64_t k) { return skipTrad.search(k); });
    auto statsSkipTrad = computeStats(skipTradBuildMs, latSkipTrad);
    printStats("Skip List Tradicional", statsSkipTrad);

    auto latSkipML = benchmarkSequential(queryKeys, [&](uint64_t k) { return skipML.search(k); });
    auto statsSkipML = computeStats(skipMLBuildMs, latSkipML);
    printStats("Skip List + ML (Base)", statsSkipML);

    auto latSkipMLOpt = benchmarkSequential(queryKeys, [&](uint64_t k) { return skipMLOpt.search(k); });
    auto statsSkipMLOpt = computeStats(skipMLOptBuildMs, latSkipMLOpt);
    printStats("Skip List + ML (Optimizado)", statsSkipMLOpt);

    // Guardar resultados JSON
    string resultsJSONPath = "dev/valerio/analysis/benchmark_results_optimized.json";
    exportResultsJSON(resultsJSONPath, statsBStarTrad, statsBStarML, statsBStarMLOpt, statsSkipTrad, statsSkipML, statsSkipMLOpt);

    // 5. Verificación de Correctitud
    cout << "\n============ VERIFICACION DE CORRECTITUD ============\n" << endl;
    bool allCorrect = true;
    for (size_t i = 0; i < queryKeys.size(); ++i) {
        uint64_t key = queryKeys[i];
        auto it = lower_bound(entries.begin(), entries.end(), key,
            [](const IndexEntry& e, uint64_t k) { return e.id < k; });
        uint64_t ref = (it != entries.end() && it->id == key) ? it->offset : 0;
        uint64_t opt1 = bStarMLOpt.search(key);
        uint64_t opt2 = skipMLOpt.search(key);
        if (ref != opt1 || ref != opt2) {
            cerr << "[ERROR] Discrepancia en clave " << key 
                 << ": ref=" << ref << " B*Opt=" << opt1 << " SkipOpt=" << opt2 << endl;
            allCorrect = false;
            break;
        }
    }
    if (allCorrect) {
        cout << "[Opt] VERIFICACION EXITOSA: B* y Skip List optimizados devuelven offsets 100% correctos." << endl;
    }

    // 6. Benchmark Paralelo (Hilos concurrentes en B* ML Optimizado)
    int numCores = static_cast<int>(thread::hardware_concurrency());
    if (numCores == 0) numCores = 4;
    cout << "\n============ BENCHMARK PARALELO (B* ML Optimizado) ============\n" << endl;
    double seqTimeMs = benchmarkParallel(queryKeys, [&](uint64_t k) { return bStarMLOpt.search(k); }, 1);
    cout << "  Secuencial (1 hilo):  " << fixed << setprecision(4) << seqTimeMs << " ms" << endl;

    vector<int> threadCounts = {2, 4};
    if (numCores >= 8) threadCounts.push_back(8);
    if (numCores > 8) threadCounts.push_back(numCores);

    vector<pair<int, double>> parallelResults;
    parallelResults.push_back({1, seqTimeMs});

    for (int nT : threadCounts) {
        double parTimeMs = benchmarkParallel(queryKeys, [&](uint64_t k) { return bStarMLOpt.search(k); }, nT);
        double speedup = seqTimeMs / parTimeMs;
        cout << "  Paralelo (" << nT << " hilos):  " << parTimeMs << " ms"
             << " | Speedup: " << setprecision(2) << speedup << "x"
             << " | QPS: " << static_cast<int>(10000.0 / (parTimeMs / 1000.0)) << endl;
        parallelResults.push_back({nT, parTimeMs});
    }

    // 7. Exportar latencias para Python
    string latPath = "dev/valerio/analysis/latencies_optimized.csv";
    ofstream latFile(latPath);
    if (latFile.is_open()) {
        latFile << "key,b_star_traditional,b_star_ml,b_star_ml_optimized,skip_list_traditional,skip_list_ml,skip_list_ml_optimized\n";
        for (size_t i = 0; i < queryKeys.size(); ++i) {
            latFile << queryKeys[i] << ","
                    << latBStarTrad[i] << ","
                    << latBStarML[i] << ","
                    << latBStarMLOpt[i] << ","
                    << latSkipTrad[i] << ","
                    << latSkipML[i] << ","
                    << latSkipMLOpt[i] << "\n";
        }
        latFile.close();
        cout << "\n[Opt] Latencias exportadas a: " << latPath << endl;
    }

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
        cout << "[Opt] Resultados de paralelismo exportados." << endl;
    }

    // 8. Consultas interactivas de comparación directa
    cout << "\n===========================================================" << endl;
    cout << "  CONSULTAS INTERACTIVAS (COMPARATIVA DE 5 ESTRUCTURAS + OPTIMIZACIONES)" << endl;
    cout << "===========================================================" << endl;
    cout << "[Interactive] Desea realizar una busqueda individual? (s/n): ";
    char choice;
    if (cin >> choice && (choice == 's' || choice == 'S')) {
        while (true) {
            cout << "\n[Interactive] Ingrese el ID a buscar (o 0 para salir): ";
            uint64_t targetId;
            if (!(cin >> targetId) || targetId == 0) break;

            auto t1 = chrono::high_resolution_clock::now();
            uint64_t o1 = bStarTrad.search(targetId);
            auto t2 = chrono::high_resolution_clock::now();
            uint64_t o2 = bStarML.search(targetId);
            auto t3 = chrono::high_resolution_clock::now();
            uint64_t o3 = bStarMLOpt.search(targetId);
            auto t4 = chrono::high_resolution_clock::now();
            uint64_t o4 = skipTrad.search(targetId);
            auto t5 = chrono::high_resolution_clock::now();
            uint64_t o5 = skipML.search(targetId);
            auto t6 = chrono::high_resolution_clock::now();
            uint64_t o6 = skipMLOpt.search(targetId);
            auto t7 = chrono::high_resolution_clock::now();

            cout << "\n----------------------------------------------------------" << endl;
            cout << "  RESULTADOS EN NANOSEGUNDOS (ns) PARA EL ID: " << targetId << endl;
            cout << "----------------------------------------------------------" << endl;
            cout << "1. Arbol B* Tradicional:         Offset=" << o1 << " | Tiempo=" << chrono::duration<double, nano>(t2-t1).count() << " ns" << endl;
            cout << "2. Arbol B* + ML (RMI Base):     Offset=" << o2 << " | Tiempo=" << chrono::duration<double, nano>(t3-t2).count() << " ns" << endl;
            cout << "3. Arbol B* + ML (RMI Optim.):   Offset=" << o3 << " | Tiempo=" << chrono::duration<double, nano>(t4-t3).count() << " ns" << endl;
            cout << "4. Skip List Tradicional:        Offset=" << o4 << " | Tiempo=" << chrono::duration<double, nano>(t5-t4).count() << " ns" << endl;
            cout << "5. Skip List + ML (Base):        Offset=" << o5 << " | Tiempo=" << chrono::duration<double, nano>(t6-t5).count() << " ns" << endl;
            cout << "6. Skip List + ML (Optim.):      Offset=" << o6 << " | Tiempo=" << chrono::duration<double, nano>(t7-t6).count() << " ns" << endl;
            cout << "----------------------------------------------------------" << endl;

            if (o1 != 0) {
                ifstream csvFile(csvPath, ios::binary);
                if (csvFile.is_open()) {
                    csvFile.seekg(o1);
                    string line;
                    if (getline(csvFile, line)) {
                        cout << "[CSV] Registro: " << line << endl;
                    }
                    csvFile.close();
                }
            } else {
                cout << "[CSV] Registro no encontrado." << endl;
            }
        }
    }

    return 0;
}

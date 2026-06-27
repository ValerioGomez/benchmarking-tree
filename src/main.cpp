#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "indexer/csv_indexer.h"
#include "b_star_tree_traditional/b_star_tree_traditional.h"
#include "b_star_tree_ml/b_star_tree_ml.h"
#include "skip_list_traditional/skip_list_traditional.h"
#include "skip_list_ml/skip_list_ml.h"

using namespace std;

// Estructura para almacenar estadísticas de benchmark
struct Stats {
    double buildTimeMs;
    double totalSearchTimeMs;
    double avgLatencyUs;
    double varianceUs;
    double stddevUs;
};

// Función para calcular estadísticas a partir de un vector de latencias en microsegundos
Stats calculateStats(double buildTime, const vector<double>& latencies) {
    double sum = 0.0;
    for (double val : latencies) {
        sum += val;
    }
    double mean = sum / latencies.size();

    double sq_sum = 0.0;
    for (double val : latencies) {
        sq_sum += (val - mean) * (val - mean);
    }
    double variance = sq_sum / latencies.size();
    double stddev = sqrt(variance);

    Stats s;
    s.buildTimeMs = buildTime;
    s.totalSearchTimeMs = sum / 1000.0; // convertir microsegundos a milisegundos
    s.avgLatencyUs = mean;
    s.varianceUs = variance;
    s.stddevUs = stddev;
    return s;
}

// Función auxiliar para exportar resultados en formato JSON
void exportResults(const string& path, const string& name, const Stats& s, bool appendComma) {
    ofstream file(path, ios::app);
    if (file.is_open()) {
        file << "  \"" << name << "\": {\n"
             << "    \"build_time_ms\": " << s.buildTimeMs << ",\n"
             << "    \"total_search_time_ms\": " << s.totalSearchTimeMs << ",\n"
             << "    \"avg_latency_us\": " << s.avgLatencyUs << ",\n"
             << "    \"variance_us\": " << s.varianceUs << ",\n"
             << "    \"stddev_us\": " << s.stddevUs << "\n"
             << "  }" << (appendComma ? "," : "") << "\n";
    }
}

int main(int argc, char* argv[]) {
    cout << "==========================================================" << endl;
    cout << "  BENCHMARKING DE INDICES APRENDIDOS VS ESTRUCTURAS TRADICIONALES" << endl;
    cout << "  Doctorado en Ciencias de la Computacion - UNA" << endl;
    cout << "==========================================================" << endl;

    string csvPath = "data/transactions_data.csv";
    string indexPath = "data/offsets.idx";
    string modelPath = "data/model_coefficients.txt";
    string resultsPath = "analysis/benchmark_results.json";

    // Modo búsqueda de ID individual si se proporciona argumento
    if (argc > 1) {
        uint64_t targetId = 0;
        try {
            targetId = stoull(argv[1]);
        } catch (...) {
            cerr << "[Main] Error: El argumento proporcionado no es un ID valido." << endl;
            return 1;
        }

        cout << "[Main] Iniciando busqueda individual para ID: " << targetId << "..." << endl;

        // Cargar el índice binario
        vector<IndexEntry> entries = CSVIndexer::loadIndex(indexPath);
        if (entries.empty()) {
            cerr << "[Main] Error al cargar el indice binario." << endl;
            return 1;
        }

        sort(entries.begin(), entries.end(), [](const IndexEntry& a, const IndexEntry& b) {
            return a.id < b.id;
        });

        // 1. Árbol B* Tradicional
        BStarTreeTraditional<uint64_t, uint64_t, 255> bStarTrad;
        bStarTrad.buildBottomUp(entries);
        uint64_t offsetBStar = bStarTrad.search(targetId);

        // 2. Árbol B* + ML
        BStarTreeML bStarML;
        if (!bStarML.loadModelsAndData(modelPath, entries)) {
            cerr << "[Main] Error al inicializar los modelos ML del Arbol B*" << endl;
            return 1;
        }
        uint64_t offsetBStarML = bStarML.search(targetId);

        // 3. Skip List Tradicional
        SkipListTraditional skipTrad(16);
        skipTrad.build(entries);
        uint64_t offsetSkip = skipTrad.search(targetId);

        // 4. Skip List + ML
        SkipListML skipML(16);
        if (!skipML.loadModel(modelPath)) {
            cerr << "[Main] Error al cargar el modelo de Skip List ML." << endl;
            return 1;
        }
        skipML.build(entries);
        uint64_t offsetSkipML = skipML.search(targetId);

        cout << "\n==============================================" << endl;
        cout << "  RESULTADOS DE BUSQUEDA PARA EL ID: " << targetId << endl;
        cout << "==============================================" << endl;
        cout << "1. Arbol B* Tradicional:  Offset = " << offsetBStar << endl;
        cout << "2. Arbol B* + ML (RMI):   Offset = " << offsetBStarML << endl;
        cout << "3. Skip List Tradicional: Offset = " << offsetSkip << endl;
        cout << "4. Skip List + ML:        Offset = " << offsetSkipML << endl;
        cout << "==============================================" << endl;

        if (offsetBStar != 0) {
            ifstream csvFile(csvPath, ios::binary);
            if (csvFile.is_open()) {
                csvFile.seekg(offsetBStar);
                string csvLine;
                if (getline(csvFile, csvLine)) {
                    cout << "\n[CSV] Registro fisico encontrado:" << endl;
                    cout << ">> " << csvLine << endl;
                }
                csvFile.close();
            } else {
                cerr << "[Main] Error: No se pudo abrir el archivo CSV para leer la linea fisica." << endl;
            }
        } else {
            cout << "\n[CSV] El ID " << targetId << " no existe en el dataset." << endl;
        }
        return 0;
    }

    // Código por defecto si no hay argumentos (Benchmark de 10,000 consultas)
    // 1. Cargar el índice binario en memoria
    auto startLoad = chrono::high_resolution_clock::now();
    vector<IndexEntry> entries = CSVIndexer::loadIndex(indexPath);
    auto endLoad = chrono::high_resolution_clock::now();
    if (entries.empty()) {
        cerr << "[Main] Error: El indice esta vacio o no se pudo cargar. Ejecute primero el indexador." << endl;
        return 1;
    }
    chrono::duration<double, milli> diffLoad = endLoad - startLoad;
    cout << "[Main] Indice cargado en: " << diffLoad.count() << " ms." << endl;

    // Asegurar que las entradas estén ordenadas por clave ID
    cout << "[Main] Verificando ordenamiento de claves..." << endl;
    sort(entries.begin(), entries.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return a.id < b.id;
    });

    // 2. Generar lote idéntico de 10,000 claves de prueba aleatorias existentes
    cout << "[Main] Generando 10,000 consultas de prueba aleatorias..." << endl;
    vector<uint64_t> queryKeys;
    queryKeys.reserve(10000);
    srand(12345); // Semilla fija para reproducibilidad
    for (int i = 0; i < 10000; ++i) {
        size_t idx = rand() % entries.size();
        queryKeys.push_back(entries[idx].id);
    }

    // Limpiar archivo de salida JSON anterior
    ofstream clearFile(resultsPath, ios::trunc);
    clearFile << "{\n";
    clearFile.close();

    // ==========================================
    // ESTRUCTURA 1: ÁRBOL B* TRADICIONAL
    // ==========================================
    cout << "\n[Benchmark] Construyendo Arbol B* Tradicional..." << endl;
    BStarTreeTraditional<uint64_t, uint64_t, 255> bStarTrad;
    auto bStarBuildStart = chrono::high_resolution_clock::now();
    bStarTrad.buildBottomUp(entries);
    auto bStarBuildEnd = chrono::high_resolution_clock::now();
    double bStarBuildMs = chrono::duration<double, milli>(bStarBuildEnd - bStarBuildStart).count();
    cout << "[Benchmark] Arbol B* Tradicional construido en: " << bStarBuildMs << " ms. Nodos: " << bStarTrad.getNodeCount() << endl;

    cout << "[Benchmark] Ejecutando 10k consultas en Arbol B* Tradicional..." << endl;
    vector<double> bStarTradLatencies;
    bStarTradLatencies.reserve(10000);
    uint64_t verificationSum = 0;
    
    for (uint64_t key : queryKeys) {
        auto q_start = chrono::high_resolution_clock::now();
        uint64_t offset = bStarTrad.search(key);
        auto q_end = chrono::high_resolution_clock::now();
        
        chrono::duration<double, micro> q_diff = q_end - q_start;
        bStarTradLatencies.push_back(q_diff.count());
        verificationSum += offset;
    }
    Stats bStarTradStats = calculateStats(bStarBuildMs, bStarTradLatencies);
    cout << " -> Latencia Promedio: " << bStarTradStats.avgLatencyUs << " us | Desv. Est.: " << bStarTradStats.stddevUs << " us" << endl;
    exportResults(resultsPath, "b_star_traditional", bStarTradStats, true);

    // ==========================================
    // ESTRUCTURA 2: ÁRBOL B* + ML (LEARNED INDEX)
    // ==========================================
    cout << "\n[Benchmark] Inicializando Arbol B* + ML (Learned Index)..." << endl;
    BStarTreeML bStarML;
    auto bStarMLBuildStart = chrono::high_resolution_clock::now();
    if (!bStarML.loadModelsAndData(modelPath, entries)) {
        cerr << "[Benchmark] Error critico: No se pudo inicializar los modelos ML para el Arbol B*" << endl;
        return 1;
    }
    auto bStarMLBuildEnd = chrono::high_resolution_clock::now();
    double bStarMLBuildMs = chrono::duration<double, milli>(bStarMLBuildEnd - bStarMLBuildStart).count();

    cout << "[Benchmark] Ejecutando 10k consultas en Arbol B* + ML..." << endl;
    vector<double> bStarMLLatencies;
    bStarMLLatencies.reserve(10000);
    uint64_t mlVerificationSum = 0;

    for (uint64_t key : queryKeys) {
        auto q_start = chrono::high_resolution_clock::now();
        uint64_t offset = bStarML.search(key);
        auto q_end = chrono::high_resolution_clock::now();
        
        chrono::duration<double, micro> q_diff = q_end - q_start;
        bStarMLLatencies.push_back(q_diff.count());
        mlVerificationSum += offset;
    }
    Stats bStarMLStats = calculateStats(bStarMLBuildMs, bStarMLLatencies);
    cout << " -> Latencia Promedio: " << bStarMLStats.avgLatencyUs << " us | Desv. Est.: " << bStarMLStats.stddevUs << " us" << endl;
    exportResults(resultsPath, "b_star_ml", bStarMLStats, true);

    if (verificationSum != mlVerificationSum) {
        cerr << "[Benchmark] ADVERTENCIA: La suma de verificacion difiere entre Arbol B* y Arbol B* + ML!" << endl;
    }

    // ==========================================
    // ESTRUCTURA 3: SKIP LIST TRADICIONAL
    // ==========================================
    cout << "\n[Benchmark] Construyendo Skip List Tradicional..." << endl;
    SkipListTraditional skipTrad(16);
    auto skipBuildStart = chrono::high_resolution_clock::now();
    skipTrad.build(entries);
    auto skipBuildEnd = chrono::high_resolution_clock::now();
    double skipBuildMs = chrono::duration<double, milli>(skipBuildEnd - skipBuildStart).count();
    cout << "[Benchmark] Skip List Tradicional construida en: " << skipBuildMs << " ms." << endl;

    cout << "[Benchmark] Ejecutando 10k consultas en Skip List Tradicional..." << endl;
    vector<double> skipTradLatencies;
    skipTradLatencies.reserve(10000);
    uint64_t skipVerificationSum = 0;

    for (uint64_t key : queryKeys) {
        auto q_start = chrono::high_resolution_clock::now();
        uint64_t offset = skipTrad.search(key);
        auto q_end = chrono::high_resolution_clock::now();
        
        chrono::duration<double, micro> q_diff = q_end - q_start;
        skipTradLatencies.push_back(q_diff.count());
        skipVerificationSum += offset;
    }
    Stats skipTradStats = calculateStats(skipBuildMs, skipTradLatencies);
    cout << " -> Latencia Promedio: " << skipTradStats.avgLatencyUs << " us | Desv. Est.: " << skipTradStats.stddevUs << " us" << endl;
    exportResults(resultsPath, "skip_list_traditional", skipTradStats, true);

    // ==========================================
    // ESTRUCTURA 4: SKIP LIST + ML (LEARNED SKIP LIST)
    // ==========================================
    cout << "\n[Benchmark] Inicializando Skip List + ML..." << endl;
    SkipListML skipML(16);
    auto skipMLBuildStart = chrono::high_resolution_clock::now();
    if (!skipML.loadModel(modelPath)) {
        cerr << "[Benchmark] Error: No se pudo cargar el modelo para la Skip List ML." << endl;
        return 1;
    }
    skipML.build(entries);
    auto skipMLBuildEnd = chrono::high_resolution_clock::now();
    double skipMLBuildMs = chrono::duration<double, milli>(skipMLBuildEnd - skipMLBuildStart).count();
    cout << "[Benchmark] Skip List + ML construida en: " << skipMLBuildMs << " ms." << endl;

    cout << "[Benchmark] Ejecutando 10k consultas en Skip List + ML..." << endl;
    vector<double> skipMLLatencies;
    skipMLLatencies.reserve(10000);
    uint64_t skipMLVerificationSum = 0;

    for (uint64_t key : queryKeys) {
        auto q_start = chrono::high_resolution_clock::now();
        uint64_t offset = skipML.search(key);
        auto q_end = chrono::high_resolution_clock::now();
        
        chrono::duration<double, micro> q_diff = q_end - q_start;
        skipMLLatencies.push_back(q_diff.count());
        skipMLVerificationSum += offset;
    }
    Stats skipMLStats = calculateStats(skipMLBuildMs, skipMLLatencies);
    cout << " -> Latencia Promedio: " << skipMLStats.avgLatencyUs << " us | Desv. Est.: " << skipMLStats.stddevUs << " us" << endl;
    exportResults(resultsPath, "skip_list_ml", skipMLStats, false);

    if (verificationSum != skipVerificationSum || verificationSum != skipMLVerificationSum) {
        cerr << "[Benchmark] ADVERTENCIA: La suma de verificacion de Skip List difiere de la de referencia!" << endl;
    } else {
        cout << "\n[Benchmark] Verificacion exitosa: Los offsets retornados por las 4 estructuras son 100% correctos." << endl;
    }

    // Cerrar archivo JSON
    ofstream closeFile(resultsPath, ios::app);
    closeFile << "}\n";
    closeFile.close();
    cout << "[Main] Resultados consolidados exportados a: " << resultsPath << endl;

    // Exportar las latencias crudas a un archivo CSV para pruebas estadísticas
    string latenciesPath = "analysis/latencies.csv";
    ofstream latFile(latenciesPath);
    if (latFile.is_open()) {
        latFile << "key,b_star_traditional,b_star_ml,skip_list_traditional,skip_list_ml\n";
        for (size_t i = 0; i < queryKeys.size(); ++i) {
            latFile << queryKeys[i] << ","
                    << bStarTradLatencies[i] << ","
                    << bStarMLLatencies[i] << ","
                    << skipTradLatencies[i] << ","
                    << skipMLLatencies[i] << "\n";
        }
        latFile.close();
        cout << "[Main] Latencias individuales exportadas a: " << latenciesPath << " para pruebas estadisticas." << endl;
    } else {
        cerr << "[Main] Error: No se pudo crear el archivo de latencias en: " << latenciesPath << endl;
    }

    // ==========================================================
    // MODO INTERACTIVO (CONSULTAS DE ID INDIVIDUALES DESDE EL CÓDIGO)
    // ==========================================================
    cout << "\n==========================================================" << endl;
    cout << "  CONSULTAS INTERACTIVAS DESDE EL CODIGO" << endl;
    cout << "==========================================================" << endl;
    cout << "[Interactive] Desea buscar IDs especificos? (s/n): ";
    char choice;
    if (cin >> choice && (choice == 's' || choice == 'S')) {
        while (true) {
            cout << "\n[Interactive] Ingrese el ID a buscar (o 0 para salir): ";
            uint64_t targetId;
            if (!(cin >> targetId) || targetId == 0) {
                break;
            }

            // 1. Árbol B* Tradicional
            auto t1 = chrono::high_resolution_clock::now();
            uint64_t offsetBStar = bStarTrad.search(targetId);
            auto t2 = chrono::high_resolution_clock::now();
            chrono::duration<double, nano> d1 = t2 - t1;

            // 2. Árbol B* + ML
            auto t3 = chrono::high_resolution_clock::now();
            uint64_t offsetBStarML = bStarML.search(targetId);
            auto t4 = chrono::high_resolution_clock::now();
            chrono::duration<double, nano> d2 = t4 - t3;

            // 3. Skip List Tradicional
            auto t5 = chrono::high_resolution_clock::now();
            uint64_t offsetSkip = skipTrad.search(targetId);
            auto t6 = chrono::high_resolution_clock::now();
            chrono::duration<double, nano> d3 = t6 - t5;

            // 4. Skip List + ML
            auto t7 = chrono::high_resolution_clock::now();
            uint64_t offsetSkipML = skipML.search(targetId);
            auto t8 = chrono::high_resolution_clock::now();
            chrono::duration<double, nano> d4 = t8 - t7;

            cout << "\n----------------------------------------------------------" << endl;
            cout << "  RESULTADOS DE BUSQUEDA EN NANOSEGUNDOS (ns) PARA ID: " << targetId << endl;
            cout << "----------------------------------------------------------" << endl;
            cout << "1. Arbol B* Tradicional:  Offset = " << offsetBStar << " | Tiempo = " << d1.count() << " ns" << endl;
            cout << "2. Arbol B* + ML (RMI):   Offset = " << offsetBStarML << " | Tiempo = " << d2.count() << " ns" << endl;
            cout << "3. Skip List Tradicional: Offset = " << offsetSkip << " | Tiempo = " << d3.count() << " ns" << endl;
            cout << "4. Skip List + ML:        Offset = " << offsetSkipML << " | Tiempo = " << d4.count() << " ns" << endl;
            cout << "----------------------------------------------------------" << endl;

            if (offsetBStar != 0) {
                ifstream csvFile(csvPath, ios::binary);
                if (csvFile.is_open()) {
                    csvFile.seekg(offsetBStar);
                    string csvLine;
                    if (getline(csvFile, csvLine)) {
                        cout << "[CSV] Registro: " << csvLine << endl;
                    }
                    csvFile.close();
                }
            } else {
                cout << "[CSV] El ID " << targetId << " no existe en el dataset." << endl;
            }
        }
    }

    return 0;
}

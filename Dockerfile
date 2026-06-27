# Dockerfile para la reproducibilidad del Benchmarking
FROM gcc:12-bullseye

# Instalar herramientas adicionales de construcción
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Establecer el directorio de trabajo en el contenedor
WORKDIR /app

# Copiar el código fuente y las carpetas estructurales
COPY src/ ./src/

# Compilar el ejecutable principal en C++17 con optimización de nivel 3 (-O3)
RUN g++ -O3 -std=c++17 src/main.cpp src/indexer/csv_indexer.cpp -o benchmark_indexes

# Definir puntos de montaje de volúmenes para vincular la data del host y los reportes
VOLUME ["/app/data", "/app/analysis"]

# Comando por defecto para ejecutar la prueba de estrés
ENTRYPOINT ["./benchmark_indexes"]

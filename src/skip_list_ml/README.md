# Skip List + Machine Learning (`src/skip_list_ml/`)

Este directorio contiene la implementación avanzada de la Skip List optimizada por modelos estadísticos/Machine Learning (Learned Skip List).

---

## 🤖 Descripción y Requerimientos

A diferencia de la Skip List clásica donde los saltos y niveles de los nodos son puramente aleatorios, este enfoque utiliza modelos estadísticos entrenados para optimizar el crecimiento y distribución de niveles de los nodos según las claves reales del dataset.

### ⚙️ Detalles Algorítmicos e Investigación:
1. **Predicción de Niveles / Saltos**: Se entrena un modelo predictivo (como regresión lineal o regresión logística simple) que evalúe la densidad de la distribución de claves locales.
2. **Determinación Matemática**: En lugar de lanzar una moneda virtual, el modelo predice cuántos saltos o niveles debe saltar un nodo basándose en la distancia a los vecinos y su densidad:
   \[ \text{Nivel} = g(\text{Densidad\_Clave}) \]
3. **Optimización**:
   * En regiones con alta concentración de transacciones, los niveles y saltos se ajustan para evitar sobrecargar comparaciones secuenciales.
   * La estructura probabilística es guiada por la distribución real de claves del dataset inmutable, maximizando la eficiencia de saltos y mejorando la localidad espacial en memoria.

# Árbol B* + Machine Learning (`src/b_star_tree_ml/`)

Este directorio contiene la implementación del Árbol B* optimizado mediante modelos de Machine Learning (Learned Index).

---

## 🤖 Descripción y Requerimientos

En este enfoque, la búsqueda jerárquica de los niveles superiores del Árbol B* se reemplaza o complementa utilizando un modelo de Machine Learning que aprende la distribución acumulativa (CDF) de las claves de indexación.

### ⚙️ Detalles Algorítmicos:
1. **Modelado Predictivo**: Se entrena un modelo regresor (como regresión lineal simple o regresión segmentada) que mapea el `Transaction_ID` a su posición física esperada:
   \[ \text{Posici\'on Predicha} = f(ID) \]
2. **Margen de Error Acotado**: El entrenamiento del modelo calcula el error máximo absoluto $\Delta$ obtenido sobre el dataset.
3. **Flujo de Búsqueda**:
   * Dado un `ID` de consulta, el modelo estima la posición: $Pos = f(ID)$.
   * Se acota el rango físico de búsqueda local al intervalo: $[Pos - \Delta, Pos + \Delta]$.
   * Se ejecuta una búsqueda binaria local rápida sobre el rango acotado en el archivo de índice del árbol B* para hallar el offset exacto.
4. **Objetivo**: Reducir sustancialmente el número de accesos a memoria/disco al acotar la búsqueda local a unos pocos bloques, en comparación con la búsqueda binaria completa en el árbol tradicional.

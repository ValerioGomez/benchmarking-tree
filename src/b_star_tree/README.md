# Árbol B* y Versión Aprendida (`src/b_star_tree/`)

Este directorio albergará la implementación de las estructuras basadas en Árbol B*.

---

## 🌲 Árbol B* Tradicional
* **Definición**: Es una variante auto-balanceada del Árbol B donde los nodos no raíz están llenos en al menos $2/3$ de su capacidad.
* **Mapeo**: Almacena las claves `ID` en los nodos hojas junto con su respectivo offset físico en bytes en el CSV.
* **Propósito**: Actuar como el baseline jerárquico estándar.

---

## 🤖 Árbol B* + Machine Learning (Learned Index)
* **Concepto**: Sustituye o complementa el recorrido jerárquico tradicional de los niveles superiores mediante un modelo predictivo.
* **Entrenamiento**: El modelo aprende la función de distribución acumulada (CDF) de las claves $F(Key) = P(X \le Key)$.
* **Búsqueda**:
  1. El modelo estima la posición del registro o página en base a la clave: $Pos = M(Key)$.
  2. Se define un rango de error acotado $[\text{Pos} - \Delta, \text{Pos} + \Delta]$.
  3. Se efectúa una búsqueda binaria local para encontrar el offset exacto dentro de ese intervalo.

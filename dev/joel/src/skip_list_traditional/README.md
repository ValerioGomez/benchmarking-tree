# Skip List Tradicional (`src/skip_list_traditional/`)

Este directorio contiene la implementación clásica de la estructura probabilística Skip List como línea base.

---

## 📝 Descripción y Requerimientos

La Skip List (Lista por Saltos) es una estructura probabilística que mantiene múltiples niveles de listas enlazadas. Permite búsquedas veloces saltándose elementos, simulando un comportamiento similar a una búsqueda binaria pero sobre listas.

### ⚙️ Detalles Algorítmicos:
1. **Asignación Probabilística**: Al insertar un elemento, la altura del nodo (cantidad de niveles que escalará) se define tirando una "moneda virtual". Sigue una distribución geométrica donde la probabilidad de subir un nivel es $p$ (comúnmente $p = 0.5$ o $p = 0.25$).
2. **Estructura de Punteros**: Cada nodo contiene un arreglo dinámico de punteros hacia adelante, representando los niveles en los que está presente.
3. **Búsqueda**: Se recorre desde el nivel más alto hacia la derecha. Si la clave siguiente es mayor, se desciende un nivel en la columna actual y se continúa, logrando un costo promedio de $O(\log n)$.

# Árbol B* Tradicional (`src/b_star_tree_traditional/`)

Este directorio contiene la implementación clásica del Árbol B* tradicional como línea base.

---

## 📝 Descripción y Requerimientos

El Árbol B* es una estructura de datos jerárquica auto-balanceada optimizada para sistemas de bases de datos y almacenamiento secundario. A diferencia de un Árbol B convencional, impone reglas más estrictas de ocupación en sus nodos.

### ⚙️ Detalles Algorítmicos:
1. **Factor de Ocupación**: Los nodos internos no raíz deben estar llenos a una capacidad mínima de **2/3** (en comparación con el 1/2 habitual del Árbol B).
2. **Redistribución en División**: Cuando un nodo se llena, intenta redistribuir claves con su nodo hermano adyacente en lugar de dividirse inmediatamente. Si ambos están llenos, se crea un tercer nodo a partir de los dos llenos (división 2-en-3).
3. **Indexación**: Cada clave `ID` (Transaction ID) se indexa en las páginas hoja apuntando a su respectivo `offset` físico en bytes dentro de `transactions_data.csv`.
4. **Búsqueda**: Complejidad garantizada de $O(\log n)$ en el peor de los casos.

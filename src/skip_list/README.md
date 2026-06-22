# Skip List y Versión Aprendida (`src/skip_list/`)

Este directorio contiene las implementaciones de las estructuras basadas en listas por saltos (Skip Lists).

---

## 📈 Skip List Tradicional
* **Definición**: Estructura de datos probabilística que mantiene múltiples listas enlazadas en niveles jerárquicos.
* **Saltos**: La altura de un nodo y los saltos se deciden mediante una distribución geométrica (azar probabilístico).
* **Propósito**: Actuar como el baseline probabilístico estándar.

---

## 🤖 Skip List + Machine Learning (Learned Skip List)
* **Concepto**: Sustituye la asignación probabilística aleatoria de niveles y saltos por un modelo de Machine Learning.
* **Modelo Predictivo**: Se entrena un regresor lineal o logístico simple con la distribución de claves del conjunto de datos.
* **Diseño**:
  * En lugar de lanzar una moneda virtual para determinar la altura de cada elemento, el modelo predictivo estima la densidad de claves locales y decide la altura del nodo de forma matemática.
  * Esto permite que las regiones del dataset con alta densidad de claves tengan saltos optimizados, mejorando el rendimiento de búsqueda en comparación con la distribución uniforme/aleatoria clásica.

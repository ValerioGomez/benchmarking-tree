# Artículo de Investigación (Academic Paper)

Esta carpeta contiene el código fuente en LaTeX, la base de datos bibliográfica y el documento final compilado en PDF para la presentación académica.

---

## 📁 Archivos en el Directorio

* `main.tex`: Estructura y cuerpo principal del artículo científico escrito en LaTeX.
* `references.bib`: Archivo BibTeX que gestiona las referencias bibliográficas y literatura científica citada en el texto.
* `report_final.pdf`: PDF compilado final listo para entrega.

---

## 🛠️ Compilación del Paper

Si realiza cambios en `main.tex` o `references.bib`, puede recompilar el documento PDF utilizando herramientas LaTeX locales.

### Opción 1: Usando `latexmk` (Recomendado)
```bash
latexmk -pdf main.tex
```

### Opción 2: Usando `pdflatex` y `bibtex` tradicional
```bash
pdflatex main.tex
bibtex main
pdflatex main.tex
pdflatex main.tex
```

### Opción 3: Editor Online (Overleaf)
1. Comprima el contenido de esta carpeta `paper/` en un archivo `.zip`.
2. Súbalo a un proyecto nuevo en [Overleaf](https://www.overleaf.com/).
3. Presione "Recompile" en el panel de Overleaf.

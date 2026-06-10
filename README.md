# 🌐 Ray Tracing Básico

Implementación fundamental de un motor de renderizado por trazado de rayos (*Ray Tracing*) desarrollado desde cero. El proyecto simula la física de la luz mediante el cálculo de intersecciones de rayos en un espacio tridimensional para generar imágenes con sombreado real, reflexiones y materiales básicos.

---

## 🚀 Características del Proyecto

* **Trazado de Rayos Primarios:** Generación de rayos desde una cámara matemática a través de un plano de imagen.
* **Cálculo de Intersecciones:** Soporte geométrico analítico para esferas [y otros objetos si los hay].
* **Modelo de Iluminación:** Simulación de sombreado basada en [menciona el modelo, ej: Phong / Lambert] con luces puntuales.
* **Efectos Visuales:** Cálculo de sombras duras/suaves y coeficientes de reflexión especular.

---

## 💻 Instalación y Ejecución

Sigue estos pasos para clonar el repositorio y ejecutar el renderizador en tu máquina local:

### Requisitos Previos
Asegúrate de tener instalado [Python/C++] y los gestores de paquetes necesarios.

### Instrucciones

```bash
# 1. Clonar el repositorio
git clone [https://github.com/Toxihacks/Ray-Tracing-Basico.git](https://github.com/Toxihacks/Ray-Tracing-Basico.git)

# 2. Acceder a la carpeta del proyecto
cd Ray-Tracing-Basico

# 3. Instalar dependencias necesarias (si aplica)
pip install -r requirements.txt

# 4. Ejecutar el script principal
python main.py

# Template básico para realizar pruebas y/o librerías en el curso de Estructuras de Datos Compactas (UdeC)

## ¿Qué posee el template?
Tienen ejemplos básicos para generar una librería, como utilizar cada carpeta, como utilizar CMakeLists. Además, viene integrado una librería de benchmark para que pueda facilitar su trabajo al momento de realizar las pruebas de rendimiento pedidas en el curso. Esta libreríá genera un csv que luego pueden plotear en su herramienta favorita (viene como ejemplo el poder graficar utilizando gnuplot, el cual debe estar instalado en su pc).

## ¿Cómo usar el template?
```
git clone https://github.com/bletelier/template-curso-cds.git
cp -r template-curso-cds mi-proyecto
cd mi-proyecto
chmod +x initialize_project.sh
./initialize_project.sh
./compile.sh
./execute_benchmarks.sh
./plot.sh
./execute_tests.sh
```
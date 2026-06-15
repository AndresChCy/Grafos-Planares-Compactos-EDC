set datafile separator ","
set encoding utf8

set terminal pdfcairo enhanced
set output "draws/fib_benchmark.pdf"

set title "Fibonacci Benchmark"
set xlabel "Input Size (n)"
set ylabel "Average Time (μs)"

set logscale y
set grid
set key outside

plot \
    "< tail -n +2 ../example_res.csv | grep ',fr,' | sort -t, -k3,3n" using 3:4:5:6 with yerrorlines pt 7 ps 0.4 title "Recursive (fr)", \
    "< tail -n +2 ../example_res.csv | grep ',fm,' | sort -t, -k3,3n" using 3:4:5:6 with yerrorlines pt 7 ps 0.4 title "Memoized (fm)", \
    "< tail -n +2 ../example_res.csv | grep ',ft,' | sort -t, -k3,3n" using 3:4:5:6 with yerrorlines pt 7 ps 0.4 title "Tabulated (ft)"
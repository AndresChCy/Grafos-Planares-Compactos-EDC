set datafile separator ","
set encoding utf8

set terminal pdfcairo enhanced
set output "draws/search_benchmark.pdf"

set title "Binary Search vs Sequential Search"
set xlabel "Input Size (n)" 
set ylabel "Average Time (μs)"

set xtics rotate by 90 right

set logscale y
set grid
set key outside

plot \
    "< tail -n +2 ../search_experiment.csv | grep ',bs,' | sort -t, -k3,3n" using 3:4:5:6 with yerrorlines pt 7 ps 0.4 title "Binary Search (bs)", \
    "< tail -n +2 ../search_experiment.csv | grep ',ls,' | sort -t, -k3,3n" using 3:4:5:6 with yerrorlines pt 7 ps 0.4 title "Sequential Search (ls)"
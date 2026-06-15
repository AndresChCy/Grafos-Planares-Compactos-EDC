set datafile separator ","
set encoding utf8

set terminal pdfcairo enhanced
set output "draws/access_benchmark.pdf"

set title "Random access avg time in a vector of size 10'000'000"
set xlabel "Accessed index"
set ylabel "Average Time (μs)"

set xtics rotate by 90 right


set grid
set key outside

plot \
    "< tail -n +2 ../random_access_10000000.csv | grep ',rand_acc,' | sort -t, -k3,3n" using 3:4 with lines title "Random Access (rand-acc)"
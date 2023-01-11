reset
clear

set title 'Resources Created'
set term png size 800,600 enhanced font "Arial,10" 
set output "resourceChart.png"
set datafile separator ","
set yrange [0:*]
set ylabel "Created"

# If we don't use columnhead, the first line of the data file
# will confuse gnuplot, which will leave gaps in the plot.
set key top left outside horizontal autotitle columnhead

set xtics rotate by 90 offset 0,0 out nomirror
set ytics out nomirror

set style fill solid border -1
# Make the histogram boxes half the width of their slots.
set boxwidth 0.5 relative

# Select histogram mode.
set style data histograms
# Select a row-stacked histogram.
set style histogram rowstacked

plot "resourceChart.csv" using 2:xticlabels(1), '' using 3, '' using 4, '' using 5, '' using 6, '' using 7, '' using 8, '' using 9

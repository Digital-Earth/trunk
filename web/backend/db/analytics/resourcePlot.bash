#!/bin/bash

if [[ $# -lt 4 ]]; then
	echo "Usage: $0 dataFile numBins imageFile.png plotTitle [width,height]"
	exit 1;
fi
 
dataFile=$1
numBins=$2
imageFile=$3
plotTitle=$4
dimensions=$5
if [[ -z $5 ]]; then
	dimensions=800,600
fi

plotFile=".tmpResourcePlot.plt"

usingString="using 2:xticlabels(1)"
for i in `seq 3 $(($numBins + 2))`; do
	usingString="$usingString, '' using $i"
done

echo "reset
clear

set title '$plotTitle'
set term png size $dimensions enhanced font 'Arial,10'
set output '$imageFile'
set datafile separator ','
set yrange [0:*]
set ylabel 'Count'

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

plot '$dataFile' $usingString" > $plotFile
gnuplot < $plotFile 2> /dev/null
rm $plotFile

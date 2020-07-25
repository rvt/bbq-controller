#!/bin/bash

function makexbm {
    filename="${1%.*}"
    echo "here $1"
    gm convert -geometry x24 -trim -flatten -background white $1 "${filename}24.xbm"
    gm convert -geometry x48 -trim -flatten -background white $1 "${filename}48.xbm"
}
 
                
#gm convert -trim -flatten -background white file.svg file.png
makexbm fan.png
makexbm thermometer.png
makexbm knob.png

#exit
for file in *.png
do
filename="${file%.*}"
echo $file
#gm convert -trim -flatten -background white $file $filename.xbm
done

for file in *.xbm
do
filename="${file%.*}"
echo $file
sed -i 's/static char/const uint8_t/g' $file
sed -i 's/\[\] \= {/[] PROGMEM = {/g' $file
done

cat base.h > ../src/icons.h
cat *.xbm >> ../src/icons.h
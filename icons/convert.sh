#!/bin/bash

shopt -s expand_aliases
set -e

exists()
{
  command -v "$1" > /dev/null 2>&1
}

exists gsed && alias sed=gsed

function makexbm {
    filename="${1%.*}"
    echo "ArrayName $1"
    gm convert -geometry x24 -trim -flatten -background white $1 "${filename}24.xbm"
    gm convert -geometry x48 -trim -flatten -background white $1 "${filename}48.xbm"
}

function makebin {
    filename="${1%.*}"
    filenameBits="${1/./_}"
    echo "Array Name $filenameBits"
    xxd -i $1 "${filename}.xxd"
}

#gm convert -trim -flatten -background white file.svg file.png
makexbm fan.png
makexbm thermometer.png
makexbm knob.png
gm convert -verbose splash240.png -depth 24 splash240.bmp
makebin splash240.bmp
#makebin parrot.bmp
#makebin parrot.bmp



for file in *.{xbm,xxd}
do
filename="${file%.*}"
echo $file
sed -i 's/static char/const PROGMEM uint8_t/g' "$file"
sed -i 's/unsigned char/const PROGMEM uint8_t/g' "$file"
sed -i 's/unsigned int/const uint32_t/g' "$file"
#sed -i 's/\[\] \= {/[] PROGMEM = {/g' "$file"
done

cat base.h > ../src/icons.h
cat *.xbm >> ../src/icons.h
cat *.xxd >> ../src/icons.h

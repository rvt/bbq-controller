

#gm convert -trim -flatten -background white file.svg file.png
#gm convert -geometry x24 -trim -flatten -background white thermometer.png thermometer.xbm
#gm convert -geometry x24 -trim -flatten -background white knob.png knob.xbm
#gm convert -geometry x24 -trim -flatten -background white fan.png fan.xbm

exit
for file in *.png
do
gm convert -trim -flatten -background white $file $file.xbm
done
cat *.xbm > images.h
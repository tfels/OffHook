 for file in *.svg; do inkscape "$file" --export-filename "${file%svg}png" --export-area=400:200:800:1000 --export-height=40; done
 
inkscape batt_no.svg --export-filename batt_no.png --export-area=300:200:900:1000 -
-export-height=40
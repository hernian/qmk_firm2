#! /usr/bin/sh
if [ ! -e generated ]; then
mkdir generated
fi
python pyutil/genbmpsrc.py images_200x200/epaper_200x200_fusion360.png generated/
python pyutil/genbmpsrc.py images_200x200/epaper_200x200_illustrator.png generated/
python pyutil/genbmpsrc.py images_200x200/epaper_200x200_kicad.png generated/
python pyutil/genbmpsrc.py images_200x200/epaper_200x200_navigation.png generated/
python pyutil/genbmpsrc.py images_200x200/epaper_200x200_numkey.png generated/
python pyutil/genbmpsrc.py images_200x200/epaper_200x200_selectmode.png generated/

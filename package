#!/bin/sh

mkdir -p opk
cp ./icon.png opk/icon.png
cp ./platform/"$1"/FAKE08 opk/FAKE08

# https://unix.stackexchange.com/questions/219268/how-to-add-new-lines-when-using-echo
print()
	case    ${IFS- } in
	(\ *)   printf  %b\\n "$*";;
	(*)     IFS=\ $IFS
	printf  %b\\n "$*"
	IFS=${IFS#?}
esac

# Create GmenuNx entry file plus other things

print '[Desktop Entry]
Type=Application
Name=FAKE08
Comment=PICO-8 Emulator (port gameblabla)
Exec=FAKE08 %f
Icon=icon
Terminal=false
Type=Application
Categories=emulators;
X-OD-NeedsDownscaling=true
X-OD-Filter=.png,.p8
selectorbrowser=true
SelectorFilter=p8,P8,png,PNG
' > opk/default."$1".desktop

mksquashfs ./opk fake08_"$1".opk -all-root -noappend -no-exports -no-xattrs

rm -r opk

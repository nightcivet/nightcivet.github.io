#!/bin/bash

if [ ! $# -eq 1 ] ;then
	echo use [cmd] [archive program desktop file]
	exit 1 
fi

if [[ ! $1 == *".desktop" ]] ;then
	echo not a desktop file
	exit 1
fi

support_mime_type=(
"application/x-tar"
"application/x-compressed-tar"
"application/x-xz-compressed-tar"
"application/x-compressed-tar"
"application/x-bzip-compressed-tar"
"application/x-tarz"
"application/x-cbr"
"application/vnd.rar"
"application/x-7z-compressed"
"application/x-java-archive"
"application/vnd.comicbook+zip"
"application/zip"
"application/x-rpm"
"image/svg+xml-compressed"
)

ls /home/ | while read line
do
	echo $line
	if [ -d /home/$line/.config ]; then
		echo find /home/$line/.config
		if [ -e /home/$line/.config/mimeapps.list ]; then
			echo find /home/$line/.config/mimeapps.list
			for mime_type in ${support_mime_type[*]}									
			do
				grep ${mime_type} /home/$line/.config/mimeapps.list > /dev/null
				if [ $? -eq 0 ] ;then
					echo "find setting ${mime_type}"
					sed -i 's#'${mime_type}'=.*#'${mime_type}'='$1'#g' /home/$line/.config/mimeapps.list
				else
					echo not setting ${mime_type}  yet
					sed -i '/\[Default Applications\]/a'${mime_type}'='$1'' /home/$line/.config/mimeapps.list
				fi
			done
		else
			echo not find /home/$line/.config/mimeapps.list
			touch /home/$line/.config/mimeapps.list
			chmod 777 /home/$line/.config/mimeapps.list
			echo [Default Applications] >> /home/$line/.config/mimeapps.list
			for mime_type in ${support_mime_type[*]}
			do
				sed -i '/\[Default Applications\]/a'${mime_type}'='$1'' /home/$line/.config/mimeapps.list
			done
		fi
	fi
done


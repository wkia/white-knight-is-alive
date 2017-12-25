# The idea is taken from https://askubuntu.com/questions/65331/how-to-convert-a-m4a-sound-file-to-mp3

import glob
import os
import re
import sys

if 2 > len(sys.argv):
	print ("Usage: %s <filemask> [<filemask> ...]" % sys.argv[0])
	exit(1)
fileext='.m4a'

sys.argv.pop(0)
filemask = sys.argv
for mask in sys.argv:
	for filename in glob.iglob(mask + "/**/*" + fileext, recursive=True):
		print(filename)
		mp3Filename = filename.replace(fileext, '.mp3')
		print(mp3Filename)

		os.system("avconv -y -i \"%s\" -f ffmetadata metadata.txt" % filename)
		#with open('metadata.txt', 'r', "utf_8_sig") as myfile:
		with open('metadata.txt', 'r', encoding="utf8") as myfile:
			metadata = myfile.read()
		
		#print (metadata)
		metadata = re.sub(r'^date=(.*)$', r'TYER=\1', metadata, flags=re.MULTILINE)
		metadata = re.sub(r'^major_brand=.*$', '', metadata, flags=re.M)
		metadata = re.sub(r'^minor_version=.*$', '', metadata, flags=re.M)
		metadata = re.sub(r'^creation.*$', '', metadata, flags=re.M)
		metadata = re.sub(r'^compatible.*$', '', metadata, flags=re.M)
		metadata = re.sub(r'^encoder=.*$', '', metadata, flags=re.M)
		#print (metadata)
		with open("metadata2.txt", "w", encoding='utf-8-sig') as myfile:
			myfile.write("%s" % metadata)

		os.system("avconv -threads auto -y -i \"%s\" -i metadata2.txt -ab 256k -map_metadata 1 -id3v2_version 3 \"%s\"" % (filename, mp3Filename))
		

import os
import subprocess
import Image
import configuration
import time

#return closest multiple of k greater than n
def multiple_k(n,k):
	return n + (k - (n%k));

map_path = configuration.MAP_DIR
tile_path = configuration.TILE_DIR
image_files = os.listdir(tile_path)
for map in os.listdir(map_path):
	if image_files.__contains__("%s.%d_%d_4.jpg" % (map,0,0)):
		continue
	print map_path
	print tile_path
	print map
	process = subprocess.Popen(["wesnoth","--screenshot",map_path+map,tile_path+map])
	deadlock = True
	for i in range(20):
		time.sleep(1)
		process.poll()
		if process.returncode != None:
			deadlock = False
			break
	if deadlock:
		os.kill(process.pid,9)
		os.remove(map_path+map)	
		continue
	im = Image.open(tile_path+"/"+map)
	#generate 1:2 scale tiles
	img_width = im.size[0]
	img_height = im.size[1]
	img_width_r = multiple_k(img_width,512)	
	img_height_r = multiple_k(img_height,512)	
	
	im_resized = Image.new(im.mode,(img_width_r,img_height_r))
	im_resized.paste(im,(0,0,img_width,img_height))
	im = im_resized
	
	width = im.size[0]
	height = im.size[1]
	
	for i in range(0,width/512):
		for j in range(0,height/512):
			tile_src = im.crop((i*512,j*512,(i+1)*512,(j+1)*512))
			tile_src = tile_src.resize((256,256))
			tile_src.MAXBLOCK = 1000000 #workaround for jpg output bug in PIL (see: http://mail.python.org/pipermail/image-sig/1999-August/000816.html)...
			tile_src.save("%s%s.%d_%d_4.jpg" % (tile_path,map,i,j),quality=70,optimize=True)
	
	#generate 1:4 scale tiles
	img_width = im.size[0]
	img_height = im.size[1]
	img_width_r = multiple_k(img_width,1024)	
	img_height_r = multiple_k(img_height,1024)	
	im_resized = Image.new(im.mode,(img_width_r,img_height_r))
	im_resized.paste(im,(0,0,img_width,img_height))
	im = im_resized
	width = im.size[0]
	height = im.size[1]

	#generate coarse zoom level, level 3
	for i in range(0,width/1024):
		for j in range(0,height/1024):
			tile_src = im.crop((i*1024,j*1024,(i+1)*1024,(j+1)*1024))
			tile_src = tile_src.resize((256,256))
			tile_src.MAXBLOCK = 1000000 #workaround for jpg output bug in PIL (see: http://mail.python.org/pipermail/image-sig/1999-August/000816.html)...
			tile_src.save("%s%s.%d_%d_3.jpg" % (tile_path,map,i,j),quality=70,optimize=True)

	#remove the intial map screenshot
	os.remove(tile_path+map)	

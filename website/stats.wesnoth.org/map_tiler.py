import os,sys
import Image

TILEWIDTH=256

def multiple_256(n):
	return n + (256 - (n%256))

for infile in sys.argv[1:]:
	try:
		im = Image.open(infile)
		img_width = im.size[0]
		img_height = im.size[1]
		#pad width and height to the nearest multiples of 256
		img_width_r = multiple_256(img_width)	
		img_height_r = multiple_256(img_height)	
		
		im_resized = Image.new(im.mode,(img_width_r,img_height_r))
		im_resized.paste(im,(0,0,img_width,img_height))
		im = im_resized
		
		width = im.size[0]
		height = im.size[1]
		filename_base = infile[0:len(infile)-3]
		#generate fine zoom level, level 4
		for i in range(0,width/256):
			for j in range(0,height/256):
				tile_src = im.crop((i*256,j*256,(i+1)*256,(j+1)*256))
				tile_src.save("%s%d_%d_4.png" % (filename_base,i,j),"PNG")
		#generate coarse zoom level, level 3
		for i in range(0,width/1024):
			for j in range(0,height/1024):
				tile_src = im.crop((i*1024,j*1024,(i+1)*1024,(j+1)*1024))
				tile_src = tile_src.resize((256,256))
				tile_src.save("%s%d_%d_3.png" % (filename_base,i,j),"PNG")
	except IOError:
		print IOError

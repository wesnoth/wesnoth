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
		for i in range(0,width/256):
			for j in range(0,height/256):
				tile = Image.new(im.mode,(256,256))
				tile_src = im.crop((i*256,j*256,(i+1)*256,(j+1)*256))
				tile.paste(tile_src,(0,0,256,256))
				tile.save("%s%d_%d.png" % (filename_base,i,j),"PNG")
	except IOError:
		print IOError

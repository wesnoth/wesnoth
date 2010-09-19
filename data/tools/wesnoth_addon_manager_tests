import os, subprocess, unittest, codecs, time, filecmp, shutil, os.path
#test.db fixture to be loaded manually.

if not os.name == 'nt': 
	wam_cmd = "./wesnoth_addon_manager.py"
else:
	wam_cmd = "wesnoth_addon_manager.py"
		
class TestClass(unittest.TestCase):
	
	def test_addonList(self):
		somefile = file("list.txt","w")
		args = ["python", wam_cmd, "-l"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile)
		somefile.close()
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("list.txt"):
			out = line
			break
		self.assertTrue('author' and 'name' and 'title' and 'downloads' in out)
		
	def test_nonexisting_addon_download(self):
		somefile = file("down_null.txt","w")
		args = ["python", wam_cmd, "-d", "sdfkln3df-foobar"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("down_null.txt"):
			out = line
			break
		self.assertTrue('No addon found' in out)
		
	def test_existing_addon_download(self):
		if os.path.exists("Brave Wings"):
			shutil.rmtree("Brave Wings")
		somefile = file("down.txt","w")
		args = ["python", wam_cmd, "-d", "Brave Wings"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		self.assertTrue(os.path.exists("Brave Wings"))
	
	def test_existing_publish(self):
		somefile = file("publish.txt","w")
		args = ["python", wam_cmd, "-u", "../test_data/game_publish_test/Brave Wings","-L", "admin", "-P", "admin"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("publish.txt"):
			out = out + line
		self.assertTrue('Addon published successfully' in out)
	
	def test_nonexisting_publish(self):
		somefile = file("publish_null.txt","w")
		args = ["python", wam_cmd, "-u", "fooooooobar"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("publish_null.txt"):
			out = line
			break
		self.assertTrue('Cannot open file fooooooobar' in out)
		
	def test_downloaded_content(self):
		if os.path.exists("Brave Wings"):
			shutil.rmtree("Brave Wings")
		somefile = file("publish.txt","w")
		args = ["python", wam_cmd, "-u", "../test_data/game_publish_test/Brave Wings","-L", "admin", "-P" ,"admin"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		
		somefile = file("down_content.txt","w")
		args = ["python", wam_cmd, "-d", "Brave Wings"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		dircomp = filecmp.dircmp("Brave Wings","../test_data/game_publish_test/Brave Wings", ignore = ['_server.pbl','_info.cfg'])
		if not dircomp.diff_files == []:
			somefile.write("Differing files: " + str(dircomp))
		self.assertTrue(dircomp.left_list == dircomp.right_list and dircomp.diff_files == [])
		
	def test_remove_published(self):
		somefile = file("remove.txt","w")
		args = ["python", wam_cmd, "-u", "../test_data/game_publish_test/Brave Wings","-L", "admin", "-P" ,"admin"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		args = ["python", wam_cmd, "-r", "Brave Wings","-L", "admin", "-P", "admin"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		args = ["python", wam_cmd, "-d", "\"Brave Wings*\""]
		proc = subprocess.Popen(args, shell = False, stdout = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		out = ''
		for line in codecs.open("down_null.txt"):
			out = line
			break
		self.assertTrue('No addon found' in out)
	
	def test_update_no_credentials(self):
		somefile = file("publish.txt","w")
		args = ["python", wam_cmd, "-u", "../test_data/game_publish_test/Brave Wings","-L", "admin", "-P", "admin"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		
		somefile = file("publish_illegal.txt","w")
		args = ["python", wam_cmd, "-u", "../test_data/game_publish_test/Brave Wings","-L", "autor2", "-P", "autor2"]
		proc = subprocess.Popen(args, shell = False, stdout = somefile, stderr = somefile)
		while proc.poll() == None:
			time.sleep(0.001)
		
		out = ''
		for line in codecs.open("publish_illegal.txt"):
			out = out + line
		self.assertTrue('This user is not one of authors' in out)
	
	#def tearDown(self):
		#if os.path.exists("Brave Wings"):
			#shutil.rmtree("Brave Wings")	
if __name__ == '__main__':
    unittest.main()

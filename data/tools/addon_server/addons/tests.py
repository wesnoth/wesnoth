from django.test import TestCase
from django.test import Client
from addons.models import *
from addons.views import *
from django.http import HttpResponse

import shutil
import tempfile 


class SimpleTest(TestCase):
	fixtures = ['testdb.json']
	#Caution: assuming certain specific content of testdb.json.
	#Any change to the test database must be explicitly reflected
	#in test code unless the test doesn't reference the database.
	#Changes the test code makes to the database are not persistent.
	def test_get_rating(self):
		a = Addon.objects.get(id=3)
		self.assertEquals(a.get_rating(), 3.0)
	
	def test_rate_www(self):
		response = self.client.post('/addons/rate/3/', {'rating' : 5}, follow=True)
		#Did server accept the rating?
		self.assertEquals(response.status_code, 200)
		#Was the correct rating received?
		self.assertEquals(response.context['rate_val'], 5)
		#Did server respond with the details template again?
		self.assertTemplateUsed(response, "addons/details.html")
		#Was the user informed he has rated successfully?
		self.assertEquals(response.context['rated'], True)
		#Was the rating updated correctly?
		a = Addon.objects.get(id=3)
		self.assertEquals(a.get_rating(), 4.0)
	
	def test_rate_by_name_wml(self):
		response = self.client.post('/addons/rate/Brave Wings/?wml', {'rating' : 5}, follow=True)
		#Did server accept the rating?
		self.assertEquals(response.status_code, 200)
		#Did server respond with the wml message template?
		self.assertTemplateUsed(response, "addons/message.wml")
		#Was the rating updated correctly?
		a = Addon.objects.get(id=11)
		self.assertEquals(a.get_rating(), 5.0)
		
	def test_rate_by_name_badval_wml(self):
		response = self.client.post('/addons/rate/Brave Wings/?wml', {'rating' : 7}, follow=True)
		#Did server accept the rating?
		self.assertEquals(response.status_code, 200)
		#Did server respond with the wml message template?
		self.assertTemplateUsed(response, "addons/error.wml")
	
	def test_addonList_wml_iface(self):
		#Test if specyfing wml iface in GET renders text output for addonList
		response = self.client.get('/addons/', {'wml' : 'true'}, follow=True)
		self.assertTemplateUsed(response, "addons/addonList.wml")
		
	def test_addonList_www_iface(self):
		#Test if not specyfing wml iface in GET renders text output for addonList
		response = self.client.get('/addons/', follow=True)
		self.assertTemplateUsed(response, "addons/addonList.html")
		
	def test_addon_details(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/details/11', follow=True)
		self.assertTemplateUsed(response, "addons/details.html")
		self.assertEquals(response.status_code, 200)
	
	def test_nonexisting_addon_details(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/details/0', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_nonexisting_addon_remove(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/remove/0', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_nonexisting_addon_rate(self):
		#Test if request for a nonexisting addon id results in 404
		response = self.client.get('/addons/rate/0', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_download_wml(self):
		response = self.client.get('/addons/download/11/?wml', follow=True)
		self.assertContains(response, '[campaign]', status_code=200)
		
	def test_download_wml_nonexisting(self):
		response = self.client.get('/addons/download/0/?wml', follow=True)
		self.assertEquals(response.status_code, 404)
	
	def test_download_www(self):
		response = self.client.get('/addons/download/11/', follow=True)
		self.assertTrue(response.content.startswith('BZ'))
	
	def test_download_by_name_wml(self):
		response = self.client.get('/addons/download/Brave Wings/?wml', follow=True)
		self.assertContains(response, '[campaign]', status_code=200)
	
	def test_download_by_name_www(self):
		response = self.client.get('/addons/download/Brave Wings/', follow=True)
		self.assertTrue(response.content.startswith('BZ'))
	
	def test_publish_fail(self):
		response = self.client.post('/addons/publish/', {'login' : 'admin', 'password' : 'admin', 'wml' : ''}, follow=True)
		self.assertEquals(response.status_code, 200)
		self.assertTemplateUsed(response, "addons/publishForm.html")
		
	
class RemoveAddon(TestCase):
	tmp_wml_file = file
	tmp_tbz_file = file
	wml_src = str
	tbz_src = str
	fixtures = ['testdb.json']
	
	def setUp(self):
		addon = Addon.objects.get(id=11)
		#copy file to safe temp storage
		self.wml_src = os.path.join(MEDIA_ROOT, str(addon.file_wml))
		self.tmp_wml_file = open('tmp_wml', 'wb')
		shutil.copy2(self.wml_src, self.tmp_wml_file.name)
		
		self.tbz_src = os.path.join(MEDIA_ROOT, str(addon.file_tbz))
		self.tmp_tbz_file = open('tmp_tbz', 'wb')
		shutil.copy2(self.tbz_src, self.tmp_tbz_file.name)
	
	def tearDown(self):
		#bring back the files
		wml_file = open(self.wml_src, 'wb')
		shutil.copy2(self.tmp_wml_file.name, self.wml_src)
		
		tbz_file = open(self.tbz_src, 'wb')
		shutil.copy2(self.tmp_tbz_file.name, self.tbz_src)
		
		wml_file.close()
		tbz_file.close()
		self.tmp_wml_file.close()
		self.tmp_tbz_file.close()
		
	def test_www_remove_nonexisting_admin(self):
		#Test if addon gets removed with admin provileges
		args = {'login' : 'admin', 'password' : 'admin'}
		response = self.client.post('/addons/remove/0/', args, follow=True)
		#Did server accept the request?
		self.assertEquals(response.status_code, 404)
		
	def test_www_remove_admin(self):
		#Test if addon gets removed with admin provileges
		args = {'login' : 'admin', 'password' : 'admin'}
		response = self.client.post('/addons/remove/11/', args, follow=True)
		#Did server accept the request?
		self.assertEquals(response.status_code, 200)
		#Did server respond with the correct template?
		self.assertTemplateUsed(response, "addons/confirmRemove.html")
		self.assertEquals(response.context['remove_success'], True)
	
	def test_wml_remove_admin(self):
		#Test if addon gets removed with admin provileges
		args = {'login' : 'admin', 'password' : 'admin'}
		response = self.client.post('/addons/remove/Brave Wings/?wml', args, follow=True)
		#Did server accept the request?
		self.assertEquals(response.status_code, 200)
		#Did server respond with the correct template?
		self.assertTemplateUsed(response, "addons/message.wml")
		
		
from django.shortcuts import render_to_response, redirect
from django.http import HttpResponse, HttpResponseServerError
from addons.models import *
from django.core.exceptions import ObjectDoesNotExist
from django.utils.datetime_safe import datetime
from django.contrib.auth import authenticate
from django.core.files import File
from django.http import Http404
from settings import MEDIA_ROOT
import logging   
import logging.handlers
import re, random, shutil, os.path, sys
from subprocess import Popen
sys.path.append("..") #TODO FIXME assume wesnoth package in PYTHONPATH?
from wesnoth.campaignserver_client import CampaignClient

import time

logger = logging.getLogger('project_logger')
logger.setLevel(logging.INFO)

LOG_FILENAME = 'log.txt'
LOG_MSG_FORMAT = "%(asctime)s - %(levelname)s - %(message)s"
handler = logging.handlers.TimedRotatingFileHandler(LOG_FILENAME, when = 'midnight')
formatter = logging.Formatter(LOG_MSG_FORMAT)
handler.setFormatter(formatter)
logger.addHandler(handler)

icons_dir = '../game/data/core/images/'
ICONS_ROOT = MEDIA_ROOT + '/icons/'

def wml_error_response(title, error):
	return render_to_response('addons/error.wml',
		{'errorType':title, 'errorDesc':error})
		
def wml_message_response(title, message):
	return render_to_response('addons/message.wml',
		{'msgTitle':title, 'msgText':message})

def index(request):
	if 'wml' in request.GET:
		t = datetime.now()
		timestamp=str(int(time.mktime(t.timetuple())))
		return render_to_response('addons/addonList.wml',
			{'addons':addonList(), 'timestamp':timestamp})
	else:
		addon_list = Addon.objects.all().order_by('-name')
		for addon in addon_list:
			try:
				addon.file_size = addon.file_tbz.size
			except (IOError, ValueError, OSError):
				addon.file_size = False
		return render_to_response('addons/addonList.html', {'addon_list': addon_list})
	

def addonList():
	addons = []

	for addon in Addon.objects.all():
		try:
			addon.filename = str(addon.file_wml)[7:]
			addon.size = str(addon.file_wml.size)

			t = addon.lastUpdate
			addon.timestamp = str(int(time.mktime(t.timetuple())))

			addons.append(addon)
		except (IOError, ValueError, OSError):
			pass

	return addons

def details(request, addon_id):
	try:
		addon = Addon.objects.get(id=addon_id)
	except Addon.DoesNotExist:
		raise Http404
	try:
		addon.file_size = addon.file_tbz.size
	except (IOError, NameError, ValueError, OSError):
		addon.file_size = False
	return render_to_response('addons/details.html', {'addon': addon})

def getFile(request, addon_id):
	logger.info("Download of addon "+str(addon_id)+" requested from "+request.META['REMOTE_ADDR']);
	try:
		addon = Addon.get_addon(addon_id)
	except (Addon.DoesNotExist):
		raise Http404
	addon.downloads = addon.downloads + 1
	addon.save()
	if 'wml' in request.GET:
		return redirect(addon.file_wml.url)
	else:
		return redirect(addon.file_tbz.url)

def rate(request, addon_id):
	try:
		addon = Addon.get_addon(addon_id)
	except (Addon.DoesNotExist):
		raise Http404
	try:
		value = int(request.POST['rating'])
		if value < 1 or value > 5:
			raise ValueError("Bad rating")
	except (KeyError, ValueError):
		if 'wml' in request.GET:
			return wml_error_response("Wrong rating value", "Wrong rating value. This may signal a game version vs. server version mismatch.")
		else:
			return HttpResponseServerError("bad rating value")	
	r = Rating()
	r.value = value
	r.ip = request.get_host()
	r.addon = addon
	r.save()
	if 'wml' in request.GET:
		return wml_message_response("Rating successful", "Thank you for rating!")
	else:
		return render_to_response('addons/details.html', {'rated' : True, 'addon_id': addon_id, 'addon': addon, 'rate_val': value})

def parse_pbl(pbl_data):
	keys_vals = {}
	num = 0
	for l in pbl_data:
		num += 1
		l = l.strip()
		m = re.match(r"^(.*)=\"(.*)\"$", l)
		if m == None:
			raise Exception('Line '+str(num)+' is invalid')
		keys_vals[m.group(1)] = m.group(2)
		
	needed_keys = ['title', 'icon', 'version', 'description', 'author', 'type']

	for key in needed_keys:
		try:
			keys_vals[key]
		except LookupError:
			raise Exception('Pbl doesn\'t have ' + key + ' key')
	return keys_vals
"""
def check_pbl():
	try:
		file_pbl = request.FILES['pbl']
	except:
		errors_pbl = True

	keys_vals = {}
	if file_pbl != None:
		try:
			keys_vals = check_pbl(file_pbl.readlines())
		except Exception as inst:
			return error_response('Pbl error', [inst.args[0]])

		try:
			addon_type = AddonType.objects.get(type_name=keys_vals['type'])
		except ObjectDoesNotExist:
			return error_response('PBL error', ['Addon has a wrong type'])

		try:
			addon = Addon.objects.get(name=keys_vals['title'])
			if len(addon.authors.filter(name=login)) == 0:
				return error_response('Author error', ['This user is not one of authors'])
		except ObjectDoesNotExist:
			pass
"""

def publish(request):
	login = request.POST['login']
	user = authenticate(username=login, password=request.POST['password'])

	if 'wml' in request.GET:
		def error_response(title, error, **kwargs):
			return wml_error_response(title, error)
	else:
		def error_response(title, error, **kwargs):
			dict = {'errorType':title, 'errorDesc':error, 'loginVal':login}
			for k in kwargs.keys(): dict[k] = kwargs[k]
			return render_to_response('addons/publishForm.html', dict)

	if user is None:
		logger.info("Attempt to login as %s from %s failed during publication"
			% (login, request.META['REMOTE_ADDR']))
		return error_response("Authentication error", "Login and/or password incorrect", errors_credentials=True);

	errors_wml = False

	try:
		file_wml = request.FILES['wml']
	except:
		file_wml = None

	cs = CampaignClient()
	if file_wml != None:
		file_data = file_wml.read().encode('ascii', 'ignore')
	else:
		if 'wml' not in request.POST:
			#print 'debug: error no wml file data'
			logger.info("Attempt to publish an addon by %s from %s failed: no WML"
				% (login, request.META['REMOTE_ADDR']))
			return error_response('File error', ['No WML file data'], errors_pbl=True)
		file_data = request.POST['wml'].encode('ascii', 'ignore')

	try:
		decoded_wml = cs.decode(file_data)
	except Exception as e:
		#print "wml decoding error: ", e
		return error_response('File error', ['WML decoding error'], errors_pbl=True)

	keys_vals = {}
	for k in ["title", "author", "description", "version", "icon", "type"]:
		keys_vals[k] = decoded_wml.get_text_val(k)
		if not keys_vals[k] is None:
			keys_vals[k] = keys_vals[k].strip()
		if keys_vals[k] is None or len(keys_vals[k]) < 1:
			#print 'debug: WML key error (PBL IN WML)'
			return error_response('WML key error', 'Mandatory key %s missing' % k)

	try:
		addon_type = AddonType.objects.get(type_name=keys_vals['type'])
	except ObjectDoesNotExist:
		return error_response('WML PBL error', ['Addon has a wrong type'], errors_pbl=True)

	try:
		addon = Addon.objects.get(name=keys_vals['title'])
		if len(addon.authors.filter(name=login)) == 0:
			return error_response('Author error', ['This user is not one of authors'], errors_author=True)
		addon.uploads += 1
	except ObjectDoesNotExist:
		addon = Addon()
		addon.name = keys_vals['title']
		addon.uploads = 1
		addon.downloads = 0

	if addon.file_tbz:
		addon.file_tbz.delete()
	if addon.file_wml:
		addon.file_wml.delete()
	if file_wml != None:
		file_wml.name = addon.name + '.wml'
	else:
		file = open(os.path.join(MEDIA_ROOT, "addons/") + addon.name + ".wml", 'wb')
		file.write(file_data)
		file.close()
		file_wml =  "addons/" + addon.name + ".wml"

	tmp_dir_name = "%016x" % random.getrandbits(128)
	cs.unpackdir(decoded_wml, tmp_dir_name, verbose = False)
	tarname = os.path.join(MEDIA_ROOT, "addons/") + addon.name + ".tar.bz2"
	#print "tar ", "cjf ", tarname, " -C ", tmp_dir_name, ' .'
	Popen(["tar", "cjf", tarname, "-C", tmp_dir_name, '.']).wait()
	shutil.rmtree(tmp_dir_name, True)
	addon.file_tbz = "addons/" + addon.name + ".tar.bz2"
	
	addon.ver = keys_vals['version']
	addon.img = keys_vals['icon']
	
	icon_path = addon.img.split('/')
	current_path = ICONS_ROOT
	i = 0
	while i<len(icon_path) - 1:
		current_path = os.path.join(current_path, icon_path[i])
		i = i + 1
		if not os.path.exists(current_path):
			os.makedirs(current_path)
	
	if not os.path.exists(ICONS_ROOT):
		os.makedirs(ICONS_ROOT)
	shutil.copyfile(icons_dir + addon.img, ICONS_ROOT + addon.img)
	addon.desc = keys_vals['description']
	addon.type = addon_type
	addon.file_wml = file_wml
	addon.lastUpdate = datetime.now()

	addon.save() #needed to avoid a "'Addon' instance needs to have a primary key 
	             # value before a many-to-many relationship can be used." error
	addon.authors.clear()
	authors = keys_vals['author'].split(",")
	if login not in authors:
		authors.append(login)
	for a in authors:
		try:
			author = Author.objects.get(name=a)
		except ObjectDoesNotExist: #todo
			author = Author(name=a)
			author.save()
		addon.authors.add(author)

	addon.save()
	logger.info("User %s from %s has successfully published addon #%d (%s)"
		% (login, request.META['REMOTE_ADDR'], addon.id, addon.name))
	if ('wml' in request.GET):
		return wml_message_response('Success', 'Addon published successfully')
	else:
		return render_to_response('addons/publishForm.html',
			{'publish_success' : True, 'loginVal' : login, 'addonId' : addon.id})

def publishForm(request):
	return render_to_response('addons/publishForm.html')
	
def removeForm(request, addon_id):
	addon = Addon.objects.get(id=addon_id)
	return render_to_response('addons/confirmRemove.html', {'addon_id':addon_id,'addon': addon})
	
def remove(request, addon_id):
	try:
		addon = Addon.get_addon(addon_id)
	except (Addon.DoesNotExist):
		if 'wml' in request.GET:
			return wml_error_response("Could not remove addon from server", "Addon not found")
		else:
			raise Http404
	
	if('login' not in request.POST or 'password' not in request.POST):
		if 'wml' in request.GET:
			return wml_error_response("Could not remove addon from server", "Login and/or password incorrect")
		else:
			return render_to_response('addons/error.html', { 'errorType':'No login or password', 'errorDesc': ['Login and/or password was not supplied']})
	logger.info("Attempt to remove addon from "+request.META['REMOTE_ADDR']);
	login = request.POST['login']
	user = authenticate(username=login, password=request.POST['password'])

	is_super = False
	if user != None:
		is_super = user.is_superuser

	addon = Addon.get_addon(addon_id)

	errors_credentials = ( user == None )
	errors_permissions = ( not is_super and
				len(addon.authors.filter(name=login)) == 0 )
	
	if not (errors_permissions or errors_credentials):
		addon.delete()
		logger.info("Addon #"+addon_id+"("+addon.name+") deleted by user "+login)
		if 'wml' in request.GET:
			return wml_message_response("Addon removed from server", "Addon was successfully removed from server")
	if (errors_credentials):
		logger.info("Attempt to login as "+login+" from "+request.META['REMOTE_ADDR']+" failed during an attempt to remove addon #"+addon_id+"("+addon.name+")");
		if 'wml' in request.GET:
			return wml_error_response("Could not remove addon from server", "Login and/or password incorrect")
	if (errors_permissions):
		logger.info("Attempt to remove addon #"+addon_id+"("+addon.name+") by "+login+" from "+request.META['REMOTE_ADDR']+" failed due to insufficient permissions.");
		if 'wml' in request.GET:
			return wml_error_response("Could not remove addon from server", "You don't have permission to remove this addon")
	return render_to_response('addons/confirmRemove.html',
							  {'addon_id':addon_id,
							   'addon': addon, 'errors_credentials':errors_credentials,
							   'errors_permissions':errors_permissions,
							   'remove_success':not(errors_credentials or errors_permissions)
							   })

def adminWescampLog(request):
	if request.user.is_staff:
		logger.info("Foobar admin reads some Wescamp logs");
		return HttpResponse("<stub> Stardate 8130: 18:30 MyAddon1, MyAddon5 sent to Wescamp")
	else:
		logger.info("Foobar noon-admin attempts to read some Wescamp logs");
		return HttpResponse("<stub> You are not an admin!")
	
def adminWescampUpdate(request):
	if request.user.is_staff:
		logger.info("Foobar admin updates some stuff@Wescamp");
		return HttpResponse("<stub> Addons that changed: MyAddon1, MyAddon5 - sent to Wescamp")
	else:
		logger.info("Foobar non-admin attempts to update some stuff@Wescamp");
		return HttpResponse("<stub> You are not an admin!")

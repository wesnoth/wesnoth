from django.shortcuts import render_to_response, redirect
from django.http import HttpResponse, HttpResponseServerError
from django.template import RequestContext
from addons.models import *
from django.core.exceptions import ObjectDoesNotExist
from django.utils.datetime_safe import datetime
from django.contrib.auth import authenticate
from django.core.files import File
from django.http import Http404
from settings import MEDIA_ROOT, WESNOTH_IMAGES_DIR, ICONS_ROOT, ADDONS_ROOT, ADDONS_DIR, ADDONS_URL
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

def get_addon_or_404(id_or_name):
	try:
		addon = Addon.get_addon(id_or_name)
	except Addon.DoesNotExist:
		raise Http404
	return addon	

def wml_error_response(title, error, request):
	return render_to_response('addons/error.wml',
		{'errorType':title, 'errorDesc':error},
		context_instance = RequestContext(request))
		
def wml_message_response(title, message, request):
	return render_to_response('addons/message.wml',
		{'msgTitle':title, 'msgText':message},
		context_instance = RequestContext(request))

def index(request):
	if 'wml' in request.GET:
		t = datetime.now()
		timestamp=str(int(time.mktime(t.timetuple())))
		return render_to_response('addons/addonList.wml',
			{'addons':addonList(), 'timestamp':timestamp},
			context_instance = RequestContext(request))
	else:
		addon_list = Addon.objects.all().order_by('-name')
		for addon in addon_list:
			try:
				addon.file_size = addon.file_tbz.size
			except (IOError, ValueError, OSError):
				addon.file_size = False
		return render_to_response('addons/addonList.html',
			{'addon_list': addon_list},
			context_instance = RequestContext(request))

def addonList():
	addons = []
	for addon in Addon.objects.all():
		addon.filename = str(addon.file_wml)[7:] #???
		try:
			addon.size = str(addon.file_wml.size)
		except (IOError, ValueError, OSError):
			addon.size = False
		t = addon.lastUpdate
		addon.timestamp = str(int(time.mktime(t.timetuple())))
		addons.append(addon)
	return addons

def details(request, addon_id):
	addon = get_addon_or_404(addon_id)
	try:
		addon.file_size = addon.file_tbz.size
	except (IOError, NameError, ValueError, OSError):
		addon.file_size = False
	return render_to_response('addons/details.html', 
		{'addon': addon},
		context_instance = RequestContext(request))

def getFile(request, addon_id):
	logger.info("Download of addon %s requested from %s" % (
		str(addon_id), request.META['REMOTE_ADDR']));
	addon = get_addon_or_404(addon_id)
	addon.downloads += 1
	addon.save()
	if 'wml' in request.GET:
		return redirect(addon.file_wml.url)
	else:
		return redirect(addon.file_tbz.url)

def rate(request, addon_id):
	addon = get_addon_or_404(addon_id)
	try:
		value = int(request.POST['rating'])
		if value < 1 or value > 5:
			raise ValueError("Bad rating")
	except (KeyError, ValueError):
		if 'wml' in request.GET:
			return wml_error_response("Wrong rating value",
			"Wrong rating value. This may signal a game/server version mismatch.", request)
		else:
			return HttpResponseServerError("bad rating value")	
	r = Rating()
	r.value = value
	r.ip = request.get_host()
	r.addon = addon
	r.save()
	if 'wml' in request.GET:
		return wml_message_response("Rating successful", "Thank you for rating!", request)
	else:
		return render_to_response('addons/details.html',
			{'rated' : True, 'addon_id': addon_id, 'addon': addon, 'rate_val': value},
			context_instance = RequestContext(request))

def parse_pbl(pbl_data):
	keys_vals = {}
	num = 0
	for line in pbl_data:
		num += 1
		line = line.strip()
		m = re.match(r"^(.*)=\"(.*)\"$", line)
		if m is None:
			raise Exception('Line %d is invalid' % num)
		keys_vals[m.group(1)] = m.group(2)
		
	required_keys = ['title', 'icon', 'version', 'description', 'author', 'type']
	for key in required_keys:
		try:
			keys_vals[key]
		except LookupError:
			raise Exception("Pbl doesn't have required key: %s" % key)
	return keys_vals

def publish(request):
	login = request.POST['login']
	if 'wml' in request.GET:
		def error_response(desc, errors=[], title = "Addon publication error"):
			return wml_error_response(title, desc, request)
	else:
		def error_response(desc, errors=[], title = "Addon publication error"):
			data = {'error_title':title, 'error_desc':desc, 'loginVal':login, 'errorFields':errors}
			for e in errors:
				data['errorField_' + e] = True
			return render_to_response('addons/publishForm.html', data,
				context_instance = RequestContext(request))

	user = authenticate(username=login, password=request.POST['password'])
	if user is None:
		logger.info("Attempt to login as %s from %s failed during publication"
			% (login, request.META['REMOTE_ADDR']))
		return error_response("Login or password incorrect", ['login']);

	try:
		file_wml = request.FILES['wml']
		file_data = file_wml.read().encode('ascii', 'ignore')
	except:
		file_wml = None
		if 'wml' not in request.POST:
			logger.info("Attempt to publish an addon by %s from %s failed: no WML"
				% (login, request.META['REMOTE_ADDR']))
			return error_response('No WML file data', ['pbl', 'zip'])
		file_data = request.POST['wml'].encode('ascii', 'ignore')
	try:
		cs = CampaignClient()
		decoded_wml = cs.decode(file_data)
	except Exception as e:
		logger.info("WML decoding error while publishing addon by %s from %s"
			% (login, request.META['REMOTE_ADDR']))
		return error_response('WML decoding error', ['pbl', 'zip'])

	pbl_info = {}
	for k in ["title", "author", "description", "version", "icon", "type"]:
		val = decoded_wml.get_text_val(k)
		if val is not None:
			val = val.strip()
		if val is None or len(val) < 1:
			logger.info("Required WML key '%s' missing while publishing addon by %s from %s"
				% (k, login, request.META['REMOTE_ADDR']))
			return error_response("Mandatory key '%s' missing" % k, ['pbl'])
		pbl_info[k] = val

	try:
		type_string = pbl_info['type']
		addon_type = AddonType.objects.get(type_name=type_string)
	except ObjectDoesNotExist:
		return error_response("Invalid addon type '%s'" % type_string, ['pbl'])

	try:
		addon = Addon.objects.get(name=pbl_info['title'])
		if len(addon.authors.filter(name=login)) == 0:
			return error_response("This user is not one of authors", ['login'])
		addon.uploads += 1
	except ObjectDoesNotExist: #new addon
		addon = Addon()
		addon.name = pbl_info['title']
		addon.uploads = 1
		addon.downloads = 0

	if addon.file_tbz:
		addon.file_tbz.delete()
	if addon.file_wml:
		addon.file_wml.delete()
	if file_wml is not None:
		file_wml.name = addon.name + '.wml'
	else:
		file = open(ADDONS_ROOT + addon.name + ".wml", 'wb')
		file.write(file_data)
		file.close()
		file_wml = ADDONS_URL + addon.name + ".wml"

	tmp_dir_name = "%016x" % random.getrandbits(128) #???
	cs = CampaignClient()
	cs.unpackdir(decoded_wml, tmp_dir_name, verbose = False)
	tarname = ADDONS_ROOT + addon.name + ".tar.bz2"
	#print "tar ", "cjf ", tarname, " -C ", tmp_dir_name, ' .'
	Popen(["tar", "cjf", tarname, "-C", tmp_dir_name, '.']).wait()
	shutil.rmtree(tmp_dir_name, True)
	addon.file_tbz = ADDONS_DIR + addon.name + ".tar.bz2"
	
	addon.ver = pbl_info['version']
	addon.desc = pbl_info['description']
	addon.type = addon_type
	addon.file_wml = file_wml
	addon.lastUpdate = datetime.now()
	addon.img = pbl_info['icon']
	
	icon_path = addon.img.split('/')
	current_path = ICONS_ROOT
	i = 0
	for i in xrange(0, len(icon_path) -1 ):
		current_path = os.path.join(current_path, icon_path[i])
		i = i + 1
		if not os.path.exists(current_path):
			os.makedirs(current_path)	
	if not os.path.exists(ICONS_ROOT):
		os.makedirs(ICONS_ROOT)
	shutil.copyfile(WESNOTH_IMAGES_DIR + addon.img, ICONS_ROOT + addon.img)

	addon.save() #needed to avoid a "'Addon' instance needs to have a primary key 
	             # value before a many-to-many relationship can be used." error
	addon.authors.clear()
	authors = pbl_info['author'].split(",")
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
	if 'wml' in request.GET:
		return wml_message_response('Success', 'Addon published successfully', request)
	else:
		return render_to_response('addons/publishForm.html',
			{'publish_success' : True, 'loginVal' : login, 'addonId' : addon.id},
			context_instance = RequestContext(request))

def publishForm(request):
	return render_to_response('addons/publishForm.html',
		context_instance = RequestContext(request))
	
def removeForm(request, addon_id):
	addon = get_addon_or_404(addon_id)
	return render_to_response('addons/confirmRemove.html',
		{'addon_id':addon_id,'addon': addon},
		context_instance = RequestContext(request))
	
def remove(request, addon_id):
	addon = get_addon_or_404(addon_id)
	if 'wml' in request.GET:
		def error_response(desc, title = "Could not remove the add-on"):
			return wml_error_response(title, desc, request)
	else:
		def error_response(desc, title = "Could not remove the add-on"):
			return render_to_response('addons/error.html', 
				{'error_title': title, 'error_desc': desc,
				'addon_id':addon_id, 'addon': addon},
				context_instance = RequestContext(request))

	if 'login' not in request.POST or 'password' not in request.POST:
		return error_response('Login or password not supplied')
	login = request.POST['login']
	logger.info("Attempt to remove addon %s by %s from %s" % (
		addon.name, login, request.META['REMOTE_ADDR']))
	user = authenticate(username=login, password=request.POST['password'])
	if user is None:
		logger.info("Login as %s from %s failed during an attempt to remove addon #%d (%s)" % (
			login, request.META['REMOTE_ADDR'], addon.id, addon.name))
		return error_response("Login or password incorrect")
	if len(addon.authors.filter(name=login)) == 0 and not user.is_superuser:
		logger.info("Remove addon #%d (%s) by %s from %s failed due to insufficient permissions" % (
			addon.id, addon.name, login, request.META['REMOTE_ADDR']))
		return error_response("You don't have sufficient permissions to remove this add-on")
	addon.delete()
	logger.info("Addon #%d (%s) deleted by user %s from %s %s" % (
		addon.id, addon.name, login, request.META['REMOTE_ADDR'], "su" if user.is_superuser else ""))
	if 'wml' in request.GET:
		return wml_message_response("Addon removed from server",
			"Addon was successfully removed from server", request)
	else:
		return render_to_response('addons/confirmRemove.html',
			{'addon_id':addon_id, 'addon': addon, 
			'remove_success':True},
			context_instance = RequestContext(request))

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


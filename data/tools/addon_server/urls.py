from django.conf.urls.defaults import *
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    # Example:
    # (r'^wesnoth_umc/', include('wesnoth_umc.foo.urls')),

    # Uncomment the admin/doc line below and add 'django.contrib.admindocs' 
    # to INSTALLED_APPS to enable admin documentation:
    # (r'^admin/doc/', include('django.contrib.admindocs.urls')),

	(r'^admin/', include(admin.site.urls)),
	(r'^addons/$', 'addons.views.index'),
	(r'^addons/details/(?P<addon_id>\d+)/$', 'addons.views.details'),
	(r'^addons/download/(?P<addon_id>.+)/$', 'addons.views.getFile'),
	(r'^addons/removeForm/(?P<addon_id>\d+)/$', 'addons.views.removeForm'),
	(r'^addons/remove/(?P<addon_id>.+)/$', 'addons.views.remove'),
	(r'^addons/rate/(?P<addon_id>.+)/$', 'addons.views.rate'),
	(r'^addons/publishForm/$', 'addons.views.publishForm'),
	(r'^addons/publish/$', 'addons.views.publish'),
	(r'^admin/wescamp-log/$', 'addons.views.adminWescampLog'),
	(r'^admin/wescamp-update/$', 'addons.views.adminWescampUpdate'),
	(r'^test_media/(?P<path>.*)$', 'django.views.static.serve',
		{'document_root': './test_media'}),
	(r'^icons/(?P<path>.*)$', 'django.views.static.serve',
		{'document_root': './test_media/icons'}),
	(r'^$', 'django.views.generic.simple.redirect_to', {'url': 'addons/'})

)

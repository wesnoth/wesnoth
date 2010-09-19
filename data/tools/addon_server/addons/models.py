from django.db import models
from django.db.models import Avg

class Author(models.Model):
	name = models.CharField(max_length=30)

	def __unicode__(self):
		return self.name	

class AddonType(models.Model):
	type_name = models.CharField(max_length=200, unique=True)
	display_name = models.CharField(max_length=200)
	def get_name(self):
		return self.display_name
	def __unicode__(self):
		return self.type_name

class Addon(models.Model):
	name = models.CharField(max_length=200, unique=True)
	img = models.CharField(max_length=200)
	ver = models.CharField(max_length=200)
	downloads = models.IntegerField(default=0)
	uploads = models.IntegerField(default=0)
	file_tbz = models.FileField(upload_to='addons')
	file_wml = models.FileField(upload_to='addons')
	type = models.ForeignKey(AddonType)
	authors = models.ManyToManyField(Author)
	desc = models.TextField()
	lastUpdate = models.DateTimeField()
	
	@staticmethod
	def get_addon(id_or_name):
		try:
			addon = Addon.objects.get(id=id_or_name)
		except (Addon.DoesNotExist, ValueError):
			addon = Addon.objects.get(name=id_or_name)
		return addon

	def get_rating(self):
		rs = self.rating_set.all()
		return rs.aggregate(Avg('value'))['value__avg']

	def __unicode__(self):
		return self.name

class Rating(models.Model):
	VALUE_CHOICES = (
		(1,'1'),
		(2,'2'),
		(3,'3'),
		(4,'4'),
		(5,'5')
	)
	value = models.IntegerField(choices=VALUE_CHOICES)
	date = models.DateField(auto_now_add=True)
	ip = models.IPAddressField()
	addon = models.ForeignKey(Addon)

	def __unicode__(self):
		return 'Rating: '+str(self.value)


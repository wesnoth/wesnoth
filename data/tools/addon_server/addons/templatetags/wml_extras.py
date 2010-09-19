from django import template
from django.template.defaultfilters import stringfilter


register = template.Library()

@register.filter
@stringfilter
def wml_escape(value):
	return '"' + value.replace('"', '""') + '"';

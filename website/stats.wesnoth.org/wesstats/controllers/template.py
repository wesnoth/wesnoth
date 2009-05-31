# -*- coding: utf-8 -*-
"""Fallback controller."""

from wesstats.lib.base import BaseController

__all__ = ['TemplateController']


class TemplateController(BaseController):
    """
    The fallback controller for wesstats.
    
    By default, the final controller tried to fulfill the request
    when no other routes match. It may be used to display a template
    when all else fails, e.g.::
    
        def view(self, url):
            return render('/%s' % url)
    
    Or if you're using Mako and want to explicitly send a 404 (Not
    Found) response code when the requested template doesn't exist::
    
        import mako.exceptions
        
        def view(self, url):
            try:
                return render('/%s' % url)
            except mako.exceptions.TopLevelLookupException:
                abort(404)
    
    """
    
    def view(self, url):
        """Abort the request with a 404 HTTP status code."""
        abort(404)

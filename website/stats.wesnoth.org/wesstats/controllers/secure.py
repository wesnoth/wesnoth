# -*- coding: utf-8 -*-
"""Sample controller with all its actions protected."""
from tg import expose, flash
from pylons.i18n import ugettext as _, lazy_ugettext as l_
from repoze.what.predicates import has_permission
#from dbsprockets.dbmechanic.frameworks.tg2 import DBMechanic
#from dbsprockets.saprovider import SAProvider

from wesstats.lib.base import BaseController
#from wesstats.model import DBSession, metadata

__all__ = ['SecureController']


class SecureController(BaseController):
    """Sample controller-wide authorization"""
    
    # The predicate that must be met for all the actions in this controller:
    allow_only = has_permission('manage',
                                msg=l_('Only for people with the "manage" permission'))
    
    @expose('wesstats.templates.index')
    def index(self):
        """Let the user know that's visiting a protected controller."""
        flash(_("Secure Controller here"))
        return dict(page='index')
    
    @expose('wesstats.templates.index')
    def some_where(self):
        """Let the user know that this action is protected too."""
        return dict(page='some_where')

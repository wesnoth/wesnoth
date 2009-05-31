# -*- coding: utf-8 -*-
"""Setup the wesstats application"""

import logging

import transaction
from tg import config

from wesstats.config.environment import load_environment

__all__ = ['setup_app']

log = logging.getLogger(__name__)


def setup_app(command, conf, vars):
    """Place any commands to setup wesstats here"""
    load_environment(conf.global_conf, conf.local_conf)
    # Load the models
    from wesstats import model
    print "Creating tables"
    model.metadata.create_all(bind=config['pylons.app_globals'].sa_engine)

    manager = model.User()
    manager.user_name = u'manager'
    manager.display_name = u'Example manager'
    manager.email_address = u'manager@somedomain.com'
    manager.password = u'managepass'

    model.DBSession.add(manager)

    group = model.Group()
    group.group_name = u'managers'
    group.display_name = u'Managers Group'

    group.users.append(manager)

    model.DBSession.add(group)

    permission = model.Permission()
    permission.permission_name = u'manage'
    permission.description = u'This permission give an administrative right to the bearer'
    permission.groups.append(group)

    model.DBSession.add(permission)

    editor = model.User()
    editor.user_name = u'editor'
    editor.display_name = u'Example editor'
    editor.email_address = u'editor@somedomain.com'
    editor.password = u'editpass'

    model.DBSession.add(editor)
    model.DBSession.flush()

    transaction.commit()
    print "Successfully setup"

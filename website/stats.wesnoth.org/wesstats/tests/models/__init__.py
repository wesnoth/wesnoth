# -*- coding: utf-8 -*-
"""Unit test suite for the models of the application."""
from nose.tools import assert_equals

from wesstats.model import DBSession
from wesstats.tests import setup_db, teardown_db

__all__ = ['ModelTest']

#Create an empty database before we start our tests for this module
def setup():
    """Function called by nose on module load"""
    setup_db()
    
#Teardown that database 
def teardown():
    """Function called by nose after all tests in this module ran"""
    teardown_db()
    
class ModelTest(object):
    """Base unit test case for the models."""
    
    klass = None
    attrs = {}

    def setup(self):
        try:
            new_attrs = {}
            new_attrs.update(self.attrs)
            new_attrs.update(self.do_get_dependencies())
            self.obj = self.klass(**new_attrs)
            DBSession.add(self.obj)
            DBSession.flush()
            return self.obj
        except:
            DBSession.rollback()
            raise

    def tearDown(self):
        DBSession.rollback()

    def do_get_dependencies(self):
        """Use this method to pull in other objects that need to be created for this object to be build properly"""
        return {}

    def test_create_obj(self):
        pass

    def test_query_obj(self):
        obj = DBSession.query(self.klass).one()
        for key, value in self.attrs.iteritems():
            assert_equals(getattr(obj, key), value)

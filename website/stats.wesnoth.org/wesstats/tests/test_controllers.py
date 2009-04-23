import unittest
import turbogears
from turbogears import testutil
from wesstats.controllers import Root
import cherrypy

cherrypy.root = Root()

class TestPages(unittest.TestCase):

    def setUp(self):
        turbogears.startup.startTurboGears()

    def tearDown(self):
        """Tests for apps using identity need to stop CP/TG after each test to
        stop the VisitManager thread.
        See http://trac.turbogears.org/turbogears/ticket/1217 for details.
        """
        turbogears.startup.stopTurboGears()

    def test_method(self):
        "the index method should return a string called now"
        import types
        result = testutil.call(cherrypy.root.index)
        assert type(result["now"]) == types.StringType

    def test_indextitle(self):
        "The indexpage should have the right title"
        testutil.create_request("/")
        response = cherrypy.response.body[0].lower()
        assert "<title>welcome to turbogears</title>" in response

    def test_logintitle(self):
        "login page should have the right title"
        testutil.create_request("/login")
        response = cherrypy.response.body[0].lower()
        assert "<title>login</title>" in response

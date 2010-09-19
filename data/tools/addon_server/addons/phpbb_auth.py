from django.conf import settings
from django.contrib.auth.models import User, check_password
from phpbb_hash import check_pswd
import MySQLdb

DB_HOST="127.0.0.1"
DB_NAME="phpbb"
DB_USER="root"
DB_PASSWORD="root"
DB_USER_TABLE="phpbb_users"

class PhpbbBackend:
	def authenticate(self, username=None, password=None):
		conn = MySQLdb.connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME)
		c = conn.cursor()
		c.execute("SELECT user_password FROM "+DB_USER_TABLE+
			  " WHERE UPPER(username)=UPPER('"+username+"')")
		if c.rowcount == 0:
			return None
		if check_pswd(password, c.fetchall()[0][0]):
			try:
				user = User.objects.get(username=username)
			except User.DoesNotExist:
					user = User(username=username, password='whatever')
					user.save()
			return user
		return None

	def get_user(self, user_id):
		try:
			return User.objects.get(pk=user_id)
		except User.DoesNotExist:
			return None

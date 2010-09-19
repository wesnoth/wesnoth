import hashlib
import random

itoa64 = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

def encode_hash(input):
	encoded = ""
	i = 0
	while (i <= 16):
		value = ord(input[i])
		i+=1
		encoded += itoa64[value & 0x3f]
		if(i < 16):
			value |= ord(input[i]) << 8
		encoded += itoa64[(value >> 6) & 0x3f]
		if(i >= 16):
			break
		i+=1
		if(i < 16):
			value |= ord(input[i]) << 16
		encoded += itoa64[(value >> 12) & 0x3f]
		if(i >= 16):
			break
		i+=1
		encoded += itoa64[(value >> 18) & 0x3f]

	return encoded

def get_iters_count(hash):
	return itoa64.index(hash[3])

def get_salt(hash):
	return hash[4:12]

def create_pepper(hash):
	return hash[0:12]

def create_salt(len):
	salt=""
	while len:
		salt+=str(random.randint(0,9))
		len-=1
	return salt


def create_hash(password, salt, iters):
	iters = 1 << iters
	out = hashlib.md5(salt + password)
	while iters > 0:
		out = hashlib.md5(out.digest()[:16]+password)
		iters -= 1
	return encode_hash(out.digest())

def check_pswd(password, hash):
	password = password.encode('ascii','ignore')
	hash = hash.encode('ascii','ignore')

	if len(hash) != 34:
		return False
	seed=create_salt(8)
	salt=create_pepper(hash)+seed
	pass_hash = create_hash(
			create_hash(password, get_salt(salt), get_iters_count(salt)),
			salt[12:20], 10)
	
	valid_hash = create_hash(hash[12:34],seed,10)
	if(pass_hash == valid_hash):
		return True

	return False

#Hash = u"$H$9rhqX7wfmZfU3xuBbAN1/nMSgNZXG2."
#print check_pswd(u"root666", Hash)

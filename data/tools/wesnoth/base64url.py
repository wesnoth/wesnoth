import base64, re

# Standard Python library provides
# base64.urlsafe_b64encode, base64.urlsafe_b64decode
# However, counter to RFC 7515, they use padding.
# https://github.com/python/cpython/issues/73613

def encode_safe(buffer):
    b64encoded = base64.b64encode(buffer)
    return b64encoded.replace(b'=', b'').replace(b'+', b'-').replace(b'/', b'_')

def decode_safe(buffer):
    b64encoded = buffer.replace(b'-', b'+').replace(b'_', b'/') + (b'=' * (len(buffer) % 4))
    return base64.b64decode(b64encoded)

def encode_str(str):
    return encode_safe(bytes(str, 'utf8')).decode('utf8')

def decode_str(str):
    return decode_safe(bytes(str, 'utf8')).decode('utf8')

def decode_match(match):
    return decode_str(match.group(0))

# OpenSSL crypto library

find_path(CRYPTO_INCLUDE_DIR openssl/md5.h)

find_library(CRYPTO_LIBRARY crypto)

# handle the QUIETLY and REQUIRED arguments and set XXX_FOUND to TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CRYPTO DEFAULT_MSG CRYPTO_LIBRARY CRYPTO_INCLUDE_DIR)

mark_as_advanced(CRYPTO_INCLUDE_DIR CRYPTO_LIBRARY)

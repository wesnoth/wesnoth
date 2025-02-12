/*
	Copyright (C) 2021 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "tls_root_store.hpp"

#include "log.hpp"

#ifdef _WIN32
#include <wincrypt.h>
#elif defined(__APPLE__)
#include <Security/Security.h>
#elif defined(__ANDROID__)
#include "filesystem.hpp"
#endif

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

namespace network_asio
{

void load_tls_root_certs(boost::asio::ssl::context &ctx)
{
#ifdef _WIN32
	HCERTSTORE hStore = CertOpenSystemStore(0, TEXT("ROOT"));
	assert(hStore != NULL);

	X509_STORE *store = X509_STORE_new();
	PCCERT_CONTEXT pContext = NULL;
	while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
		X509 *x509 = d2i_X509(NULL,
			const_cast<const unsigned char**>(&pContext->pbCertEncoded),
			pContext->cbCertEncoded);
		if(x509 != NULL) {
			X509_STORE_add_cert(store, x509);
			X509_free(x509);
		}
	}

	CertFreeCertificateContext(pContext);
	CertCloseStore(hStore, 0);

	SSL_CTX_set_cert_store(ctx.native_handle(), store);
#elif defined(__APPLE__)
	X509_STORE *store = X509_STORE_new();
	CFArrayRef certs = NULL;
	// copy all system certs
	OSStatus os_status = SecTrustCopyAnchorCertificates(&certs);

	// check for any problems copying the certs
	if(os_status != 0) {
		ERR_NW << "Error enumerating certificates.";

		if (certs != NULL) {
			CFRelease(certs);
		}
		return;
	}

	for(CFIndex i = 0; i < CFArrayGetCount(certs); i++) {
		SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(certs, i);

		// convert the cert to DER format
		CFDataRef der_cert = SecCertificateCopyData(cert);
		if(!der_cert) {
			ERR_NW << "Error getting a DER representation of a certificate.";
			continue;
		}

		// decode each cert to an openssl X509 object
		const uint8_t* der_cert_ptr = CFDataGetBytePtr(der_cert);
		X509* x509_cert = d2i_X509(NULL, &der_cert_ptr, CFDataGetLength(der_cert));
		if(!x509_cert) {
			ERR_NW << "Error deciding the X509 certificate.";
			CFRelease(der_cert);
			continue;
		}

		// Add the X509 openssl object to the verification store
		if(X509_STORE_add_cert(store, x509_cert) != 1) {
			CFRelease(der_cert);
			X509_free(x509_cert);
			ERR_NW << "Error adding the X509 certificate to the store.";
			continue;
		}
	}

	CFRelease(certs);
	SSL_CTX_set_cert_store(ctx.native_handle(), store);
#elif defined(__ANDROID__)
	ctx.load_verify_file(game_config::path +  "/certificates/cacert.pem");
#else
	ctx.set_default_verify_paths();
#endif
}

}

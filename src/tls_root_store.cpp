#include "tls_root_store.hpp"

#ifdef _WIN32
#include <wincrypt.h>
#endif

namespace network_asio
{

void load_tls_root_certs(boost::asio::ssl::context &ctx)
{
#ifdef _WIN32
	HCERTSTORE hStore = CertOpenSystemStore(0, "ROOT");
	assert(hStore != NULL);

	X509_STORE *store = X509_STORE_new();
	PCCERT_CONTEXT pContext = NULL;
	while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
		X509 *x509 = d2i_X509(NULL,
			(const unsigned char **)&pContext->pbCertEncoded,
			pContext->cbCertEncoded);
		if(x509 != NULL) {
			X509_STORE_add_cert(store, x509);
			X509_free(x509);
		}
	}

	CertFreeCertificateContext(pContext);
	CertCloseStore(hStore, 0);

	SSL_CTX_set_cert_store(ctx.native_handle(), store);
#else
	ctx.set_default_verify_paths();
#endif
}

}

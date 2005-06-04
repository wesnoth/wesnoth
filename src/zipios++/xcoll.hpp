#ifndef XZIPIOS_XCOLL_H
#define XZIPIOS_XCOLL_H

#include <zipios++/collcoll.h>

#define stubmethod(rtype,name,proto) rtype name
namespace xzipios {

	class XCColl: public zipios::CColl {
	public:
		// explicit XCColl
		bool hasSubdir(const std::string) const ;
		void childrenOf(std::string path,
		                std::vector<std::string>* files,
		                std::vector<std::string>* dirs) const;
	};
}

#endif // XZIPIOS_XCOLL_H

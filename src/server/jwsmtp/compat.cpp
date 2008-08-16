// Note that the only valid version of the GPL as far as jwSMTP
// is concerned is v2 of the license (ie v2, not v2.2 or v3.x or whatever),
// unless explicitly otherwise stated.
//
// This file is part of the jwSMTP library.
//
//  jwSMTP library is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 of the License.
//
//  jwSMTP library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with jwSMTP library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// jwSMTP library
//   http://johnwiggins.net
//   smtplib@johnwiggins.net
//

#include <string>
#include <vector>
#include "compat.h"

namespace jwsmtp {

bool Connect(SOCKET sockfd, const SOCKADDR_IN& addr) {
#ifdef WIN32
   return bool(connect(sockfd, (sockaddr*)&addr, addr.get_size()) != SOCKET_ERROR);
#else
   return bool(connect(sockfd, (sockaddr*)&addr, addr.get_size()) == 0);
#endif
}

bool Socket(SOCKET& s, int domain, int type, int protocol) {
   s = socket(AF_INET, type, protocol);
#ifdef WIN32
   return bool(s != INVALID_SOCKET);
#else
   return bool(s != -1);
#endif   
}

bool Send(int &CharsSent, SOCKET s, const char *msg, size_t len, int flags) {
   CharsSent = send(s, msg, len, flags);
#ifdef WIN32
	return bool((CharsSent != SOCKET_ERROR));
#else
	return bool((CharsSent != -1));
#endif   
}

bool Recv(int &CharsRecv, SOCKET s, char *buf, size_t len, int flags) {
   CharsRecv = recv(s, buf, len, flags);
#ifdef WIN32
	return bool((CharsRecv != SOCKET_ERROR));
#else
	return bool((CharsRecv != -1));
#endif  
}

// just wrap the call to shutdown the connection on a socket
// this way I don't have to call it this way everywhere.
void Closesocket(const SOCKET& s) {
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
}

// This does nothing on unix.
// for windoze only, to initialise networking, snore
void initNetworking() {
#ifdef WIN32
	class socks
	{
	public:
		bool init() {

			WORD wVersionRequested;
			WSADATA wsaData;

			wVersionRequested = MAKEWORD( 2, 0 );
			int ret = WSAStartup( wVersionRequested, &wsaData);
			if(ret)
				return false;
			initialised = true;
			return true;
		}
		bool IsInitialised() const {return initialised;}
		socks():initialised(false){init();}
		~socks()
		{
			if(initialised)
				shutdown();
		}
	private:
		void shutdown(){WSACleanup();}
		bool initialised;
	};
	static socks s;
#endif
}

} // end namespace jwsmtp


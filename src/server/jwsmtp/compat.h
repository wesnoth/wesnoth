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

#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifdef WIN32
#pragma warning( disable : 4786 )
// tell the linker which libraries to find functions in
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#else // assume some unix variant
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET; // get round windows definitions.
#endif

//windows:
//struct sockaddr_in {
//    short sin_family;
//    unsigned short sin_port;
//    struct   in_addr sin_addr;
//    char sin_zero[8];
//};
//
//struct   in_addr {
//    union   {
//        struct{
//             unsigned  char   s_b1,
//                              s_b2,
//                              s_b3,
//                              s_b4;
//        }  S_un_b;
//        struct  {
//             unsigned  short  s_w1,
//                              s_w2;
//        }  S_un_w;
//        unsigned long  S_addr;
//     } S_un;
//};
//
//struct sockaddr {
//    u_short sa_family;
//    char    sa_data[14];
//};

//linux:
//struct sockaddr_in {
//   short int sin_family; /* AF_INET */
//  unsigned short int sin_port; /* Port number */
//  struct in_addr sin_addr; /* Internet address */
//};
//
//struct in_addr {
//  unsigned long int s_addr;
//};

namespace jwsmtp {

struct SOCKADDR_IN {
  sockaddr_in ADDR; // we are wrapping this structure.

  // this is bad as we just assume that the address "addr" is valid here.
  // need a real check and set ok appropriately  
//  SOCKADDR_IN(sockaddr_in addr):ADDR(addr), ok(true) {}
  
  SOCKADDR_IN(const std::string& address, unsigned short port, short family = AF_INET) {
    ADDR.sin_port = port;
    ADDR.sin_family = family;
#ifdef WIN32
    ADDR.sin_addr.S_un.S_addr = inet_addr(address.c_str());
    ok = (ADDR.sin_addr.S_un.S_addr != INADDR_NONE);    
#else
    ok = (inet_aton(address.c_str(), &ADDR.sin_addr));
#endif
  }

  SOCKADDR_IN(const SOCKADDR_IN& addr) {
    ADDR = addr.ADDR;
    ok = addr.ok;
  }
  SOCKADDR_IN operator = (const SOCKADDR_IN& addr) {
    ADDR = addr.ADDR;
    ok = addr.ok;
    return *this;
  }

  operator bool() const {return ok;}

  operator const sockaddr_in () const {
    return ADDR;
  }
  operator const sockaddr () const {
    sockaddr addr;
    std::copy((char*)&ADDR, (char*)&ADDR + sizeof(ADDR), (char*)&addr);
    return addr;
  }

  size_t get_size() const {return sizeof(ADDR);}
  char* get_sin_addr() {
     return (char*)&ADDR.sin_addr;
  }
  void set_port(unsigned short newport) {ADDR.sin_port = newport;}
  void set_ip(const std::string& newip) {
#ifdef WIN32
    ADDR.sin_addr.S_un.S_addr = inet_addr(newip.c_str());
    ok = (ADDR.sin_addr.S_un.S_addr != INADDR_NONE);
#else
    ok = (inet_aton(newip.c_str(), &ADDR.sin_addr));
#endif
  }
  void zeroaddress() {
#ifdef WIN32
      ADDR.sin_addr.S_un.S_addr = 0;
#else
      ADDR.sin_addr.s_addr = 0;
#endif
  }
  
private:
  bool ok;
};

bool Connect(SOCKET sockfd, const SOCKADDR_IN& addr);
bool Socket(SOCKET& s, int domain, int type, int protocol);
bool Send(int &CharsSent, SOCKET s, const char *msg, size_t len, int flags);
bool Recv(int &CharsRecv, SOCKET s, char *buf, size_t len, int flags);
void Closesocket(const SOCKET& s);
void initNetworking();

} // end namespace jwsmtp

#endif //  #ifndef __COMPAT_H__




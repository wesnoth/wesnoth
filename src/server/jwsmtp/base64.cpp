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
// base64.h: base64 encoding functions
//

#include <vector>
#include <string>
#include "base64.h"

namespace jwsmtp {

char getbase64character(const char& in)
{
	switch(in) {
		case B64_A:
			return 'A';
		case B64_B:
			return 'B';
		case B64_C:
			return 'C';
		case B64_D:
			return 'D';
		case B64_E:
			return 'E';
		case B64_F:
			return 'F';
		case B64_G:
			return 'G';
		case B64_H:
			return 'H';
		case B64_I:
			return 'I';
		case B64_J:
			return 'J';
		case B64_K:
			return 'K';
		case B64_L:
			return 'L';
		case B64_M:
			return 'M';
		case B64_N:
			return 'N';
		case B64_O:
			return 'O';
		case B64_P:
			return 'P';
		case B64_Q:
			return 'Q';
		case B64_R:
			return 'R';
		case B64_S:
			return 'S';
		case B64_T:
			return 'T';
		case B64_U:
			return 'U';
		case B64_V:
			return 'V';
		case B64_W:
			return 'W';
		case B64_X:
			return 'X';
		case B64_Y:
			return 'Y';
		case B64_Z:
			return 'Z';
		case B64_a:
			return 'a';
		case B64_b:
			return 'b';
		case B64_c:
			return 'c';
		case B64_d:
			return 'd';
		case B64_e:
			return 'e';
		case B64_f:
			return 'f';
		case B64_g:
			return 'g';
		case B64_h:
			return 'h';
		case B64_i:
			return 'i';
		case B64_j:
			return 'j';
		case B64_k:
			return 'k';
		case B64_l:
			return 'l';
		case B64_m:
			return 'm';
		case B64_n:
			return 'n';
		case B64_o:
			return 'o';
		case B64_p:
			return 'p';
		case B64_q:
			return 'q';
		case B64_r:
			return 'r';
		case B64_s:
			return 's';
		case B64_t:
			return 't';
		case B64_u:
			return 'u';
		case B64_v:
			return 'v';
		case B64_w:
			return 'w';
		case B64_x:
			return 'x';
		case B64_y:
			return 'y';
		case B64_z:
			return 'z';
		case B64_0:
			return '0';
		case B64_1:
			return '1';
		case B64_2:
			return '2';
		case B64_3:
			return '3';
		case B64_4:
			return '4';
		case B64_5:
			return '5';
		case B64_6:
			return '6';
		case B64_7:
			return '7';
		case B64_8:
			return '8';
		case B64_9:
			return '9';
		case plus:
			return '+';
		case slash:
			return '/';
		case padding:
			return '=';
	}
	return '\0'; // ?????? yikes
}

std::vector<char> base64encode(const std::vector<char>& input, const bool returns) {
   std::vector<char> output;

   // add a newline (SMTP demands less than 1000 characters in a message line).
   long count = 0;
   for(std::vector<char>::size_type p = 0; p < input.size(); p+=3) {
      output.push_back(getbase64character((input[p] & 0xFC) >> 2));
      ++count;

      if(p+1 < input.size()) {
         output.push_back(getbase64character(((input[p] & 0x03) <<4) | ((input[p+1] & 0xF0) >> 4)));
         ++count;
      }
      if(p+2 < input.size()) {
         output.push_back(getbase64character(((input[p+1] & 0x0F) <<2) | ((input[p+2] & 0xC0) >>6)));
         output.push_back(getbase64character((input[p+2] & 0x3F)));
         ++count;
      }

      if(p+1 == input.size()) {
         output.push_back(getbase64character(((input[p] & 0x03) <<4)));
      }
      else if(p+2 == input.size()) {
         output.push_back(getbase64character(((input[p+1] & 0x0F) <<2)));
      }

      if(returns) {
         // 79 characters on a line.
         if(count > 75) {
            output.push_back('\r');
            output.push_back('\n');
            count = 0;
         }
      }
   }

   int pad(input.size() % 3);
   if(pad) {
      if(pad == 1)
         pad = 2;
      else
         pad = 1;
   }
   for(int i = 0; i < pad; ++i)
      output.push_back('=');

   return output;
}

std::string base64encode(const std::string& input, const bool returns) {
   std::vector<char> in, out;
   for(std::string::const_iterator it = input.begin(); it != input.end(); ++it) {
      in.push_back(*it);
   }
   out = base64encode(in, returns);
   std::string ret;
   for(std::vector<char>::const_iterator it1 = out.begin(); it1 != out.end(); ++it1) {
      ret += *it1;
   }
   return ret;
}

} // end namespace jwsmtp



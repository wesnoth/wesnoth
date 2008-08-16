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

#ifndef __BASE64_H__
#define __BASE64_H__

namespace jwsmtp {

// added the B64 to all members of the enum for SunOS (thanks Ken Weinert)
enum BASE64
{
	B64_A, B64_B, B64_C, B64_D, B64_E, B64_F, B64_G, B64_H, B64_I, B64_J, B64_K, B64_L, B64_M, B64_N, B64_O, B64_P, B64_Q, B64_R, B64_S, B64_T, B64_U, B64_V, B64_W, B64_X, B64_Y, B64_Z,
	B64_a, B64_b, B64_c, B64_d, B64_e, B64_f, B64_g, B64_h, B64_i, B64_j, B64_k, B64_l, B64_m, B64_n, B64_o, B64_p, B64_q, B64_r, B64_s, B64_t, B64_u, B64_v, B64_w, B64_x, B64_y, B64_z,
	B64_0, B64_1, B64_2, B64_3, B64_4, B64_5, B64_6, B64_7, B64_8, B64_9, plus, slash, padding
};

char getbase64character(const char& in);
std::vector<char> base64encode(const std::vector<char>& input, const bool returns = true);
std::string base64encode(const std::string& input, const bool returns = true);

} // end namespace jwsmtp


#endif //  #ifndef __BASE64_H__


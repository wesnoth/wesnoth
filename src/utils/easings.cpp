//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
//
// For more information, please refer to <https://unlicense.org>
//

#include "utils/easings.hpp"

#include <boost/math/constants/constants.hpp>
using namespace boost::math::constants;

#include <cmath>

namespace utils::easing
{
// Modeled after the line y = x
double linear_interpolation(double p)
{
	return p;
}

// Modeled after the parabola y = x^2
double quadratic_ease_in(double p)
{
	return p * p;
}

// Modeled after the parabola y = -x^2 + 2x
double quadratic_ease_out(double p)
{
	return -(p * (p - 2));
}

// Modeled after the piecewise quadratic
// y = (1/2)((2x)^2)             ; [0, 0.5)
// y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
double quadratic_ease_in_out(double p)
{
	if(p < 0.5) {
		return 2 * p * p;
	} else {
		return (-2 * p * p) + (4 * p) - 1;
	}
}

// Modeled after the cubic y = x^3
double cubic_ease_in(double p)
{
	return p * p * p;
}

// Modeled after the cubic y = (x - 1)^3 + 1
double cubic_ease_out(double p)
{
	double f = (p - 1);
	return f * f * f + 1;
}

// Modeled after the piecewise cubic
// y = (1/2)((2x)^3)       ; [0, 0.5)
// y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
double cubic_ease_in_out(double p)
{
	if(p < 0.5) {
		return 4 * p * p * p;
	} else {
		double f = ((2 * p) - 2);
		return 0.5 * f * f * f + 1;
	}
}

// Modeled after the quartic x^4
double quartic_ease_in(double p)
{
	return p * p * p * p;
}

// Modeled after the quartic y = 1 - (x - 1)^4
double quartic_ease_out(double p)
{
	double f = (p - 1);
	return f * f * f * (1 - p) + 1;
}

// Modeled after the piecewise quartic
// y = (1/2)((2x)^4)        ; [0, 0.5)
// y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
double quartic_ease_in_out(double p)
{
	if(p < 0.5) {
		return 8 * p * p * p * p;
	} else {
		double f = (p - 1);
		return -8 * f * f * f * f + 1;
	}
}

// Modeled after the quintic y = x^5
double quintic_ease_in(double p)
{
	return p * p * p * p * p;
}

// Modeled after the quintic y = (x - 1)^5 + 1
double quintic_ease_out(double p)
{
	double f = (p - 1);
	return f * f * f * f * f + 1;
}

// Modeled after the piecewise quintic
// y = (1/2)((2x)^5)       ; [0, 0.5)
// y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
double quintic_ease_in_out(double p)
{
	if(p < 0.5) {
		return 16 * p * p * p * p * p;
	} else {
		double f = ((2 * p) - 2);
		return 0.5 * f * f * f * f * f + 1;
	}
}

// Modeled after quarter-cycle of sine wave
double sine_ease_in(double p)
{
	return std::sin((p - 1) * half_pi<double>()) + 1;
}

// Modeled after quarter-cycle of sine wave (different phase)
double sine_ease_out(double p)
{
	return std::sin(p * half_pi<double>());
}

// Modeled after half sine wave
double sine_ease_in_out(double p)
{
	return 0.5 * (1 - std::cos(p * pi<double>()));
}

// Modeled after shifted quadrant IV of unit circle
double circular_ease_in(double p)
{
	return 1 - std::sqrt(1 - (p * p));
}

// Modeled after shifted quadrant II of unit circle
double circular_ease_out(double p)
{
	return std::sqrt((2 - p) * p);
}

// Modeled after the piecewise circular function
// y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
// y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
double circular_ease_in_out(double p)
{
	if(p < 0.5) {
		return 0.5 * (1 - std::sqrt(1 - 4 * (p * p)));
	} else {
		return 0.5 * (std::sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
	}
}

// Modeled after the exponential function y = 2^(10(x - 1))
double exponential_ease_in(double p)
{
	return (p == 0.0) ? p : std::pow(2, 10 * (p - 1));
}

// Modeled after the exponential function y = -2^(-10x) + 1
double exponential_ease_out(double p)
{
	return (p == 1.0) ? p : 1 - std::pow(2, -10 * p);
}

// Modeled after the piecewise exponential
// y = (1/2)2^(10(2x - 1))         ; [0,0.5)
// y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
double exponential_ease_in_out(double p)
{
	if(p == 0.0 || p == 1.0)
		return p;

	if(p < 0.5) {
		return 0.5 * std::pow(2, (20 * p) - 10);
	} else {
		return -0.5 * std::pow(2, (-20 * p) + 10) + 1;
	}
}

// Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
double elastic_ease_in(double p)
{
	return sin(13 * half_pi<double>() * p) * std::pow(2, 10 * (p - 1));
}

// Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
double elastic_ease_out(double p)
{
	return std::sin(-13 * half_pi<double>() * (p + 1)) * std::pow(2, -10 * p) + 1;
}

// Modeled after the piecewise exponentially-damped sine wave:
// y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
// y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
double elastic_ease_in_out(double p)
{
	if(p < 0.5) {
		return 0.5 * std::sin(13 * half_pi<double>() * (2 * p)) * std::pow(2, 10 * ((2 * p) - 1));
	} else {
		return 0.5 * (std::sin(-13 * half_pi<double>() * ((2 * p - 1) + 1)) * std::pow(2, -10 * (2 * p - 1)) + 2);
	}
}

// Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
double back_ease_in(double p)
{
	return p * p * p - p * std::sin(p * pi<double>());
}

// Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
double back_ease_out(double p)
{
	double f = (1 - p);
	return 1 - (f * f * f - f * std::sin(f * pi<double>()));
}

// Modeled after the piecewise overshooting cubic function:
// y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
// y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
double back_ease_in_out(double p)
{
	if(p < 0.5) {
		double f = 2 * p;
		return 0.5 * (f * f * f - f * std::sin(f * pi<double>()));
	} else {
		double f = (1 - (2 * p - 1));
		return 0.5 * (1 - (f * f * f - f * std::sin(f * pi<double>()))) + 0.5;
	}
}

double bounce_ease_in(double p)
{
	return 1 - bounce_ease_out(1 - p);
}

double bounce_ease_out(double p)
{
	if(p < 4 / 11.0) {
		return (121 * p * p) / 16.0;
	} else if(p < 8 / 11.0) {
		return (363 / 40.0 * p * p) - (99 / 10.0 * p) + 17 / 5.0;
	} else if(p < 9 / 10.0) {
		return (4356 / 361.0 * p * p) - (35442 / 1805.0 * p) + 16061 / 1805.0;
	} else {
		return (54 / 5.0 * p * p) - (513 / 25.0 * p) + 268 / 25.0;
	}
}

double bounce_ease_in_out(double p)
{
	if(p < 0.5) {
		return 0.5 * bounce_ease_in(p * 2);
	} else {
		return 0.5 * bounce_ease_out(p * 2 - 1) + 0.5;
	}
}

} // end namespace utils::easing

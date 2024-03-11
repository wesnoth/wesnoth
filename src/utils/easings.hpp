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

#pragma once

namespace utils::easing
{
// Linear interpolation (no easing)
double linear_interpolation(double p);

// Quadratic easing; p^2
double quadratic_ease_in(double p);
double quadratic_ease_out(double p);
double quadratic_ease_in_out(double p);

// Cubic easing; p^3
double cubic_ease_in(double p);
double cubic_ease_out(double p);
double cubic_ease_in_out(double p);

// Quartic easing; p^4
double quartic_ease_in(double p);
double quartic_ease_out(double p);
double quartic_ease_in_out(double p);

// Quintic easing; p^5
double quintic_ease_in(double p);
double quintic_ease_out(double p);
double quintic_ease_in_out(double p);

// Sine wave easing; sin(p * PI/2)
double sine_ease_in(double p);
double sine_ease_out(double p);
double sine_ease_in_out(double p);

// Circular easing; sqrt(1 - p^2)
double circular_ease_in(double p);
double circular_ease_out(double p);
double circular_ease_in_out(double p);

// Exponential easing, base 2
double exponential_ease_in(double p);
double exponential_ease_out(double p);
double exponential_ease_in_out(double p);

// Exponentially-damped sine wave easing
double elastic_ease_in(double p);
double elastic_ease_out(double p);
double elastic_ease_in_out(double p);

// Overshooting cubic easing;
double back_ease_in(double p);
double back_ease_out(double p);
double back_ease_in_out(double p);

// Exponentially-decaying bounce easing
double bounce_ease_in(double p);
double bounce_ease_out(double p);
double bounce_ease_in_out(double p);

} // end namespace utils::easing

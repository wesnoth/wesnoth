#include <limits>

// This test verifies that floating point numbers are represented in the IEEE 754 format.
// Wesnoth requires that.
int main()
{
	// Return code zero means success.
	// Thus, check that the bit representation is *not* what IEEE 754 specifies.
	bool match = std::numeric_limits<double>::is_iec559;
	return !match;
}

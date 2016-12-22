#include <cinttypes>
#include <cstdint>
#include <cstdio>

// This test verifies that floating point numbers are represented in the IEEE 754 format.
// Wesnoth requires that.
int main()
{
	union
	{
		double floating_point_number;
		uint64_t integer;
	} number;

	number.floating_point_number = 1.2;
	bool match = (number.integer == 0x3FF3333333333333ull);
	if (!match)
	{
		std::printf("Wrong binary representation. Expected 0x3FF3333333333333, got 0x%" PRIX64 "\n",
		            number.integer);
	}
	// Return code zero means success.
	// Thus, check that the bit representation is *not* what IEEE 754 specifies.
	return !match;
}

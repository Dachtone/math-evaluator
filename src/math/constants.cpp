#include "internals.h"

#include <cmath>
#include <vector>

std::vector<std::pair<std::string, MathInternals::NumberType>> MathInternals::g_vConstants =
{

	/* Mathematical constants */
	{ "pi", static_cast<MathInternals::NumberType>(std::acos(-1.0)) },
	// e, uppercase to avoid name collision with exp()
	{ "e",  static_cast<MathInternals::NumberType>(std::exp(1.0)) },
	// Golden ratio
	{ "phi",  static_cast<MathInternals::NumberType>((1.0 + std::sqrt(5.0)) / 2.0) }

	/* Physical constants */
/*
	// Avogadro constant
	{ "N_A", static_cast<MathInternals::NumberType>(6.02214076e23) },
	{ "c", static_cast<MathInternals::NumberType>(299792458.0) }
*/

};
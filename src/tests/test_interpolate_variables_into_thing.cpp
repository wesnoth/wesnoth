/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <boost/test/unit_test.hpp>

#include "log.hpp"
#include "tstring.hpp"
#include "formula_string_utils.hpp"

#include <boost/bind.hpp>


/*
./test --report_level=detailed --log_level=all --run_test=interpolate_suite

 */

BOOST_AUTO_TEST_SUITE( interpolate_suite )

namespace{
	void check_equal_interpolation(std::string const & a, std::string const & correct, utils::string_map & symbols){
		std::string ans = utils::interpolate_variables_into_string(a, &symbols);
		BOOST_CHECK_MESSAGE(ans == correct, "\nInterpolation \""<< ans <<"\"" << "\nshould be     \""<<correct<<"\"\n");
	}
	void check_not_equal_interpolation(std::string const & a, std::string const & correct, utils::string_map & symbols){
		std::string ans = utils::interpolate_variables_into_string(a, &symbols);
		BOOST_CHECK_MESSAGE(ans != correct, "\nInterpolation \""<< ans <<"\"" << "\nshould not be  \""<<correct<<"\"\n");
	}
}

BOOST_AUTO_TEST_CASE( test_1 ) {
#ifdef DEBUG
	//	lg::set_log_domain_severity("interpolate", 0); //4 is debug comments
#endif
	utils::string_map symbols;
	std::string nothing = "no change";
	check_equal_interpolation(nothing, nothing, symbols);

	std::string checkL = "$simple";
	symbols["simple"] = "check";
	std::string checkR = "check";
	check_equal_interpolation(checkL, checkR, symbols);

	//Nesting
	std::string	nestingL = "ne$second$third|to|dolls";
	symbols["second"] = "BBB";
	symbols["secondTTTto"] = "sting_";
	symbols["third"] = "TTT";
	std::string	nestingR = "nesting_dolls";
	check_equal_interpolation(nestingL, nestingR, symbols);

	//Recursion1
	std::string	recursiveL = "recursive$dog";
	symbols["dog"] = "_inner_$expansion_";
	symbols["expansion_"] = "cat";
	std::string	recursiveR = "recursive_inner_$expansion_";
	check_equal_interpolation(recursiveL, recursiveR, symbols);
	//$index
	std::string	replaceIndexL ="vector[$index].tail";
	symbols["index"] = "77";
	std::string	replaceIndexR ="vector[77].tail";
	check_equal_interpolation(replaceIndexL, replaceIndexR, symbols);

	std::string	vectorElementL ="$vector2[$index].tail";
	symbols["vector2[77].tail"] = "Vector Element";
	std::string	vectorElementR ="Vector Element";
	check_equal_interpolation(vectorElementL, vectorElementR, symbols);

	std::string	vectorElement3L ="Missing Fieldname $vector3[$index].#extrastuff";
	symbols["vector3[77].tail"] = "Vector Element";
	std::string	vectorElement3R ="Missing Fieldname vector3[77].#extrastuff";
	//Note this should also report a wml error
	check_equal_interpolation(vectorElement3L, vectorElement3R, symbols);

	std::string	tailendS = "tailend$";
	check_equal_interpolation(tailendS, tailendS, symbols);

	std::string	periodd="period.";
	check_equal_interpolation(periodd, periodd, symbols);

	std::string	doubledotL = "s$double..$dot";
	symbols["double"] = "ee_double";
	symbols["dot"] = "dots";
	std::string	doubledotR = "see_double..dots";
	check_equal_interpolation(doubledotL, doubledotR, symbols);

	std::string	 tripledotsL="s$triple...$dot";
	symbols["triple"] = "ee_three";
	std::string	tripledotsR ="see_three...dots";
	check_equal_interpolation(tripledotsL, tripledotsR, symbols);

	std::string	redcheckL = "$$doubleS|S|_check";
	symbols["doubleS"] = "double";
	std::string	redcheckR ="double_check";
	check_equal_interpolation(redcheckL, redcheckR, symbols);

	std::string	embedSandspaceL = "embed$|dollar and space";
	std::string	embedSandspaceR = "embed$dollar and space";
	check_equal_interpolation(embedSandspaceL, embedSandspaceR, symbols);

	std::string	embed_and_parseL = "stop$embed$|dollar";
	symbols["embed$dollar"] = "_stuff";
	symbols["embed"] = "_parse_at_embedded_";
	std::string	embed_and_parseR = "stop_parse_at_embedded_$dollar";
	check_equal_interpolation(embed_and_parseL, embed_and_parseR, symbols);

	std::string	invalid_dollar_barL = "invalid$$$|_construction";
	std::string	invalid_dollar_barR = "invalid$_construction";
	check_equal_interpolation(invalid_dollar_barL, invalid_dollar_barR, symbols);

	std::string	stop_embedded_ctrlL = "stop$embed*star";
	symbols["embed*dollar"] = "_noway";
	symbols["embed"] = "_parse_at_embedded_";
	std::string	stop_embedded_ctrlR = "stop_parse_at_embedded_*star";
	check_equal_interpolation(stop_embedded_ctrlL, stop_embedded_ctrlR, symbols);

	std::string	stop_recursive_embedded_ctrlL = "stop$fly$embed|star";
	symbols["fly"] = "_ERROR_";
	symbols["flyweight"] = "_recursive__parse_";
	symbols["embed"] = "weight*at_embedded_";
	symbols["flyweight*at_embedded_star"] = "_never";
	//std::string	stop_recursive_embedded_ctrlR = "stop_recursive_parse_*at_embedded_star";
	std::string	stop_recursive_embedded_ctrlR = "stop_never";
	check_equal_interpolation(stop_recursive_embedded_ctrlL, stop_recursive_embedded_ctrlR, symbols);

	std::string	invalidcharsL = "invalid#va$FFFF^char@acte&rs*\tintersper+=sed";
	symbols["FFFF"] = "riable";
	std::string	invalidcharsR = "invalid#variable^char@acte&rs*\tintersper+=sed";
	check_equal_interpolation(invalidcharsL, invalidcharsR, symbols);


	//Formula Tests
	std::string	basic_formulaL = "1+2=$(1+2)";
	std::string	basic_formulaR = "1+2=3";
	check_equal_interpolation(basic_formulaL, basic_formulaR, symbols);

	std::string	embed_var_in_formulaL = "44 - 1=$($formula_var1 - 1)";
	std::string	embed_var_in_formulaR = "44 - 1=43";
	symbols["formula_var1"] = "44";
	check_equal_interpolation(embed_var_in_formulaL, embed_var_in_formulaR, symbols);

	std::string	nest_paren_in_formulaL = "44 - 12 = $($formula_var1 - (3*4))";
	std::string	nest_paren_in_formulaR = "44 - 12 = 32";
	check_equal_interpolation(nest_paren_in_formulaL, nest_paren_in_formulaR, symbols);

	std::string	comment_in_formulaL = "44 - 16 = $($formula_var1 - (4*4) #)'Hidden #)";
	std::string	comment_in_formulaR = "44 - 16 = 28";
	check_equal_interpolation(comment_in_formulaL, comment_in_formulaR, symbols);

	std::string	string_in_formulaL = "4 = $(['four_dogs' -> 4] ['four_dogs'])";
	std::string	string_in_formulaR = "4 = 4";
	check_equal_interpolation(string_in_formulaL, string_in_formulaR, symbols);

	std::string	mismatch_paren_in_formulaL = "44 - 12 = $($formula_var1 - (3*4())";
	std::string	mismatch_paren_in_formulaR = "44 - 12 = (44 - (3*4())";
	check_equal_interpolation(mismatch_paren_in_formulaL, mismatch_paren_in_formulaR, symbols);

	std::string	mismatch_comment_in_formulaL = "44 - 16 = $($formula_var1 - (4*4) #)'Hidden # #)";
	std::string	mismatch_comment_in_formulaR = "44 - 16 = (44 - (4*4) #)'Hidden # #)";
	check_equal_interpolation(mismatch_comment_in_formulaL, mismatch_comment_in_formulaR, symbols);

	std::string	mismatch_string_in_formulaL = "4 = $(['four_dogs' -> 4] ['four_'dogs'])";
	std::string	mismatch_string_in_formulaR = "4 = (['four_dogs' -> 4] ['four_'dogs'])";
	check_equal_interpolation(mismatch_string_in_formulaL, mismatch_string_in_formulaR, symbols);


}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()


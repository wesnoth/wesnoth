/*
	Copyright (C) 2012 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include "help/help_impl.hpp"

BOOST_AUTO_TEST_SUITE( help_markup )

BOOST_AUTO_TEST_CASE( test_simple_help_markup )
{
	// This tests markup with no tags.
	config output;

	// Text parses as text
	output = help::parse_text("Hello World");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "Hello World");

	// Backslashes protect the following character even if it's special
	output = help::parse_text(R"==(\<not_a_tag\>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "<not_a_tag>");

	// Simple named character entities are substituted
	output = help::parse_text("Me &amp; You");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "Me & You");

	// Decimal character entities work for single-byte characters
	output = help::parse_text("&#198;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "\u00c6");

	// Hex character entities work for single-byte characters
	output = help::parse_text("&#xc6;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "\u00c6");

	// Decimal character entities work for two-byte characters
	output = help::parse_text("&#5792;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "\u16a0");

	// Hex character entities work for two-byte characters
	output = help::parse_text("&#x16a0;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "\u16a0");

	// Decimal character entities work for non-BMP characters
	output = help::parse_text("&#128519;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "\U0001f607");

	// Hex character entities work for non-BMP characters
	output = help::parse_text("&#x1f607;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "\U0001f607");

	// Single newlines are taken literally
	output = help::parse_text("Line One\nLine Two");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "Line One\nLine Two");

	// Double newlines split into paragraphs
	output = help::parse_text("Paragraph One\n\nParagraph Two");
	BOOST_CHECK_EQUAL(output.all_children_count(), 2);
	BOOST_CHECK_EQUAL(output.child_count("text"), 2);
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "Paragraph One");
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 1)["text"], "Paragraph Two");

	// TODO: What about triple, quadruple newlines?

	// Unknown named character entities are processed but not substituted
	output = help::parse_text("This &entity; is unknown!");
	BOOST_CHECK_EQUAL(output.all_children_count(), 3);
	BOOST_CHECK_EQUAL(output.child_count("text"), 2);
	BOOST_CHECK(output.has_child("character_entity"));
	BOOST_CHECK_EQUAL(output.find_total_first_of("character_entity"), 1);
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "This ");
	BOOST_CHECK(output.mandatory_child("text", 1).has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 1)["text"], " is unknown!");
	BOOST_CHECK(output.mandatory_child("character_entity").has_attribute("name"));
	BOOST_CHECK_EQUAL(output.mandatory_child("character_entity")["name"], "entity");

	// A backslash at end-of-stream is literal
	output = help::parse_text(R"==(Ending with backslash\)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], R"==(Ending with backslash\)==");

	// A backslash can escape itself
	output = help::parse_text(R"==(Backslash\\in middle)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], R"==(Backslash\in middle)==");

	// A backslash is removed even if the escaped character is not special
	output = help::parse_text(R"==(\T\h\i\s is \p\o\i\n\t\l\e\s\s)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "This is pointless");

	// The other four simple named character entities are substituted
	output = help::parse_text("&quot;&lt;tag attr=&apos;val&apos;&gt;&quot;");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("text"));
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], R"==("<tag attr='val'>")==");
}

BOOST_AUTO_TEST_CASE( test_help_markup_old )
{
	// This tests strings using old-style help markup tags
	config output;

	// A simple tag with text content
	// This format is both old-style and new-style.
	output = help::parse_text("<tt>some text</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");

	// With explicit text attribute
	output = help::parse_text("<tt>text='some text'</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");

	// With implicit text attribute and another attribute
	output = help::parse_text("<tt>attr='value' some text</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// With explict text attribute and another attribute
	output = help::parse_text("<tt>attr='value' text='some text'</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// A tag in a larger span of text
	output = help::parse_text("Here we have <tt>attr='value' text='some text'</tt> with an unknown style applied!");
	BOOST_CHECK_EQUAL(output.all_children_count(), 3);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK_EQUAL(output.child_count("text"), 2);
	BOOST_CHECK_EQUAL(output.find_total_first_of("tt"), 1);
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "Here we have ");
	BOOST_CHECK(output.mandatory_child("text", 1).has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 1)["text"], " with an unknown style applied!");

	// The attribute value can be unquoted
	output = help::parse_text("<tt>attr=value</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// Nonalphanumeric characters don't need to be quoted as long as they're not special
	output = help::parse_text("<tt>attr=!@#$%^</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "!@#$%^");

	// Quoting with single quotes
	output = help::parse_text("<tt>attr='value with spaces'</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value with spaces");

	// Quoting with double quotes
	output = help::parse_text(R"==(<tt>attr="value with spaces"</tt>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value with spaces");

	// Quotes only count as quotes if they're the first non-whitespace character after the =
	output = help::parse_text("<tt>attr=O'Brien</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "O'Brien");

	// Single quotes in double-quoted value
	output = help::parse_text(R"==(<tt>attr="'tis futile"</tt>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "'tis futile");

	// Double quotes in single-quoted value
	output = help::parse_text(R"==(<tt>attr='the "mega" test'</tt>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], R"==(the "mega" test)==");

	// Spaces around the equals are allowed
	output = help::parse_text("<tt>attr = value</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// Newlines are also allowed
	output = help::parse_text("<tt>attr=\nvalue\nthat=\nthis</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("that"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["that"], "this");

	// Escaping a single quote in a single-quoted value
	output = help::parse_text(R"==(<tt>attr='Let\'s go?'</tt>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "Let's go?");

	// Using a simple character entity in a single-quoted value
	output = help::parse_text("<tt>attr='&apos;tis good'</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "'tis good");

	// A newline in a single-quoted value
	output = help::parse_text("<tt>attr='Line 1\nLine 2'</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "Line 1\nLine 2");

	// Using a simple character entity in a double-quoted value
	output = help::parse_text(R"==(<tt>attr="&quot;Yes!&quot;"</tt>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], R"==("Yes!")==");
}

BOOST_AUTO_TEST_CASE( test_help_markup_new )
{
	// This tests strings using new-style help markup tags
	config output;

	// A simple tag with text content
	// This format is both old-style and new-style.
	output = help::parse_text("<tt>some text</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");

	// A simple auto-closed tag
	output = help::parse_text("<tt/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(!output.mandatory_child("tt").has_attribute("text"));

	// Auto-closed tag can have a space before the slash
	output = help::parse_text("<tt />");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(!output.mandatory_child("tt").has_attribute("text"));

	// With an attribute
	output = help::parse_text("<tt attr='value'>some text</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// With an attribute that has no value specified
	output = help::parse_text("<tt attr>some text</tt>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "");

	// A tag in a larger span of text
	output = help::parse_text("Here we have <tt attr='value'>some text</tt> with an unknown style applied!");
	BOOST_CHECK_EQUAL(output.all_children_count(), 3);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK_EQUAL(output.child_count("text"), 2);
	BOOST_CHECK_EQUAL(output.find_total_first_of("tt"), 1);
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["text"], "some text");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "Here we have ");
	BOOST_CHECK(output.mandatory_child("text", 1).has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 1)["text"], " with an unknown style applied!");

	// The attribute value can be unquoted
	output = help::parse_text("<tt attr=value/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// Nonalphanumeric characters don't need to be quoted as long as they're not special
	output = help::parse_text("<tt attr=!@#$%^/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "!@#$%^");

	// Quoting with single quotes
	output = help::parse_text("<tt attr='value with spaces'/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value with spaces");

	// Quoting with double quotes
	output = help::parse_text(R"==(<tt attr="value with spaces"/>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value with spaces");

	// Quotes only count as quotes if they're the first non-whitespace character after the =
	output = help::parse_text("<tt attr=O'Brien/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "O'Brien");

	// Single quotes in double-quoted value
	output = help::parse_text(R"==(<tt attr="'tis futile"/>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "'tis futile");

	// Double quotes in single-quoted value
	output = help::parse_text(R"==(<tt attr='the "mega" test'/>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], R"==(the "mega" test)==");

	// Spaces around the equals are allowed
	output = help::parse_text("<tt attr = value/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");

	// Newlines are also allowed
	output = help::parse_text("<tt attr=\nvalue\nthat=\nthis/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "value");
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("that"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["that"], "this");

	// Escaping a single quote in a single-quoted value
	output = help::parse_text(R"==(<tt  attr='Let\'s go?'/>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "Let's go?");

	// Using a simple character entity in a single-quoted value
	output = help::parse_text("<tt attr='&apos;tis good'/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "'tis good");

	// A newline in a single-quoted value
	output = help::parse_text("<tt attr='Line 1\nLine 2'/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], "Line 1\nLine 2");

	// Using a simple character entity in a double-quoted value
	output = help::parse_text(R"==(<tt attr="&quot;Yes!&quot;"/>)==");
	BOOST_CHECK_EQUAL(output.all_children_count(), 1);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK(output.mandatory_child("tt").has_attribute("attr"));
	BOOST_CHECK_EQUAL(output.mandatory_child("tt")["attr"], R"==("Yes!")==");

	// Tags can be nested
	output = help::parse_text("We like to <tt>nest <abc>various</abc> <def>tags</def> within</tt> each other!");
	BOOST_CHECK_EQUAL(output.all_children_count(), 3);
	BOOST_CHECK(output.has_child("tt"));
	BOOST_CHECK_EQUAL(output.child_count("text"), 2);
	BOOST_CHECK_EQUAL(output.find_total_first_of("tt"), 1);
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "We like to ");
	BOOST_CHECK(output.mandatory_child("text", 1).has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 1)["text"], " each other!");
	// Simplify the remaining tests for this one by eliminating the outer layer
	output = output.mandatory_child("tt");
	BOOST_CHECK_EQUAL(output.all_children_count(), 5);
	BOOST_CHECK(output.has_child("abc"));
	BOOST_CHECK(output.has_child("def"));
	BOOST_CHECK_EQUAL(output.child_count("text"), 3);
	BOOST_CHECK_EQUAL(output.find_total_first_of("abc"), 1);
	BOOST_CHECK_EQUAL(output.find_total_first_of("def"), 3);
	BOOST_CHECK(output.mandatory_child("text").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text")["text"], "nest ");
	BOOST_CHECK(output.mandatory_child("abc").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("abc")["text"], "various");
	BOOST_CHECK(output.mandatory_child("text", 1).has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 1)["text"], " ");
	BOOST_CHECK(output.mandatory_child("def").has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("def")["text"], "tags");
	BOOST_CHECK(output.mandatory_child("text", 2).has_attribute("text"));
	BOOST_CHECK_EQUAL(output.mandatory_child("text", 2)["text"], " within");

	// Two tags with nothing between them shouldn't have an intervening text span.
	output = help::parse_text("<img src=help/orb-green.png align=here/><img src=help/orb-green.png align=there/>");
	BOOST_CHECK_EQUAL(output.all_children_count(), 2);
	BOOST_CHECK_EQUAL(output.child_count("img"), 2);
	BOOST_CHECK(output.mandatory_child("img").has_attribute("src"));
	BOOST_CHECK_EQUAL(output.mandatory_child("img")["src"], "help/orb-green.png");
	BOOST_CHECK(output.mandatory_child("img").has_attribute("align"));
	BOOST_CHECK_EQUAL(output.mandatory_child("img")["align"], "here");
	BOOST_CHECK(output.mandatory_child("img", 1).has_attribute("src"));
	BOOST_CHECK_EQUAL(output.mandatory_child("img", 1)["src"], "help/orb-green.png");
	BOOST_CHECK(output.mandatory_child("img", 1).has_attribute("align"));
	BOOST_CHECK_EQUAL(output.mandatory_child("img", 1)["align"], "there");
}

BOOST_AUTO_TEST_SUITE_END()

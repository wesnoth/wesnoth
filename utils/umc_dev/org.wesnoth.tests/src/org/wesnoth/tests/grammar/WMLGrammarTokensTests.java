package org.wesnoth.tests.grammar;

import org.wesnoth.tests.WMLTests;

/**
 * Tests parts of the grammar
 */
public class WMLGrammarTokensTests extends WMLTests
{
    // for convenience, define constants for the
    // rule names in your grammar
    // the names of terminal rules will be capitalised
    // and "RULE_" will be appended to the front
    private static final String ID         = "RULE_ID";
    private static final String SL_COMMENT = "RULE_SL_COMMENT";

    public void testID( )
    {
        checkTokenisation( "1", ID );
        checkTokenisation( "a", ID );
        checkTokenisation( "abc", ID );
        checkTokenisation( "abc123", ID );
        checkTokenisation( "abc_123", ID );
    }

    public void testSLCOMMENT( )
    {
        checkTokenisation( "#comment", SL_COMMENT );
        checkTokenisation( "#comment\n", SL_COMMENT );
        checkTokenisation( "# comment \t\t comment\r\n", SL_COMMENT );
    }

    public void testTokenParsing( )
    {
        checkParsing( "amount=+$random\r\n", grammar_.getWMLKeyRule( ) );
        checkParsing( "name={VALUE}\n", grammar_.getWMLKeyRule( ) );
        checkParsing( "{NAME}={VALUE}\n", grammar_.getWMLKeyRule( ) );

        failParsing( "name=value", grammar_.getWMLKeyRule( ) );
        failParsing( "name=\n", grammar_.getWMLKeyRule( ) );
    }
}

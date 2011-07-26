package org.wesnoth.tests.grammar;

import org.wesnoth.tests.WMLTests;

/**
 * Tests parts of the grammar
 */
public class WMLGrammarTokens extends WMLTests
{
    // for convenience, define constants for the
    // rule names in your grammar
    // the names of terminal rules will be capitalised
    // and "RULE_" will be appended to the front
    private static final String ID = "RULE_ID";
    private static final String WS = "RULE_WS";
    private static final String SL_COMMENT = "RULE_SL_COMMENT";

    public void testID()
    {
        checkTokenisation( "1", ID );
        checkTokenisation( "a", ID );
        checkTokenisation( "abc", ID );
        checkTokenisation( "abc123", ID );
        checkTokenisation( "abc_123", ID );
    }

    public void testSLCOMMENT()
    {
        checkTokenisation( "#comment", SL_COMMENT );
        checkTokenisation( "#comment\n", SL_COMMENT );
        checkTokenisation( "# comment \t\t comment\r\n", SL_COMMENT );
    }

    public void testTokenSequences()
    {
        checkParsing( "amount=+$random\r\n", grammar.getWMLKeyRule( ) );

        checkTokenisation( "123 abc", ID, WS, ID );
        checkTokenisation( "123 \t#comment\n abc", ID, WS, SL_COMMENT, WS, ID );
        // note that no white space is necessary!
        checkTokenisation( "123abc", ID );
    }
}
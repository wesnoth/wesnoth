package org.wesnoth.tests;

import org.wesnoth.WMLStandaloneSetup;

public class WMLTestsImpl extends WMLTests
{
	@SuppressWarnings("rawtypes")
	@Override
	Class getStandaloneSetupClass() {
		return WMLStandaloneSetup.class;
	}

	//for convenience, define constants for the
	//rule names in your grammar
	//the names of terminal rules will be capitalised
	//and "RULE_" will be appended to the front
	private static final String ID="RULE_ID"; //$NON-NLS-1$
	private static final String INT="RULE_INT"; //$NON-NLS-1$
	private static final String WS="RULE_WS"; //$NON-NLS-1$
	private static final String SL_COMMENT="RULE_SL_COMMENT"; //$NON-NLS-1$

	public void testINT()
	{
		checkTokenisation("1", INT); //$NON-NLS-1$
	}

	public void testID(){
		checkTokenisation("a", ID); //$NON-NLS-1$
		checkTokenisation("abc", ID); //$NON-NLS-1$
		checkTokenisation("abc123", ID); //$NON-NLS-1$
		checkTokenisation("abc_123", ID); //$NON-NLS-1$
	}
	public void testSLCOMMENT(){
		checkTokenisation("#comment", SL_COMMENT); //$NON-NLS-1$
		checkTokenisation("#comment\n", SL_COMMENT); //$NON-NLS-1$
		checkTokenisation("# comment \t\t comment\r\n", SL_COMMENT); //$NON-NLS-1$
	}

	public void testTokenSequences(){
		showTokenisation("amount=+$random\n"); //$NON-NLS-1$
		checkParsing("amount=+$random", "WMLKey"); //$NON-NLS-1$ //$NON-NLS-2$

		checkTokenisation("123 abc", ID, WS, ID); //$NON-NLS-1$
		checkTokenisation("123 \t#comment\n abc", ID, WS, SL_COMMENT,WS,ID); //$NON-NLS-1$
		//note that no white space is necessary!
		checkTokenisation("123abc", ID); //$NON-NLS-1$
	}

}
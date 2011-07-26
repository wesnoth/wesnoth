/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests;

import junit.framework.TestSuite;

import org.junit.Test;
import org.wesnoth.tests.grammar.WMLFilesTests;
import org.wesnoth.tests.grammar.WMLGrammarTokensTests;

public class WMLTestsSuite
{
    @Test
    public static TestSuite suite( )
    {
        TestSuite suite = new TestSuite( );

        // grammar
        suite.addTestSuite( WMLFilesTests.class );
        suite.addTestSuite( WMLGrammarTokensTests.class );

        return suite;
    }
}

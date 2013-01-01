/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.formatting;

import org.eclipse.xtext.formatting.impl.AbstractDeclarativeFormatter;
import org.eclipse.xtext.formatting.impl.FormattingConfig;

/**
 * This class contains custom formatting description.
 * 
 * see : http://www.eclipse.org/Xtext/documentation/latest/xtext.html#formatting
 * on how and when to use it
 * 
 * Also see {@link org.eclipse.xtext.xtext.XtextFormattingTokenSerializer} as an
 * example
 */
public class WMLFormatter extends AbstractDeclarativeFormatter
{
    @Override
    protected void configureFormatting( FormattingConfig c )
    {
        org.wesnoth.services.WMLGrammarAccess f = ( org.wesnoth.services.WMLGrammarAccess ) getGrammarAccess( );

        // disable autoline-wrap for now
        c.setAutoLinewrap( 500 );

        // no space after '[' and '[/'
        c.setNoSpace( ).after(
            f.getWMLTagAccess( ).getLeftSquareBracketKeyword_0( ) );
        c.setNoSpace( ).after(
            f.getWMLTagAccess( ).getLeftSquareBracketSolidusKeyword_5( ) );

        // no space before and after ']'
        c.setNoSpace( ).around(
            f.getWMLTagAccess( ).getRightSquareBracketKeyword_3( ) );
        c.setNoSpace( ).around(
            f.getWMLTagAccess( ).getRightSquareBracketKeyword_7( ) );

        // no space before and after the '=' in 'key=value'
        c.setNoSpace( ).around( f.getWMLKeyAccess( ).getEqualsSignKeyword_1( ) );

        // one indentation after tag ...
        c.setIndentationIncrement( ).before(
            f.getWMLTagAccess( ).getExpressionsAssignment_4( ) );

        // but get back the [/<tagname>]
        c.setIndentationDecrement( ).before(
            f.getWMLTagAccess( ).getLeftSquareBracketSolidusKeyword_5( ) );

        c.setLinewrap( 0, 1, 2 ).before( f.getSL_COMMENTRule( ) );
    }
}

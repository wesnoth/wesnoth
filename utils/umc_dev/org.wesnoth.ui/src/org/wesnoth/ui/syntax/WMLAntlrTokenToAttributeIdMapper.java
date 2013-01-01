/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import org.eclipse.xtext.ui.editor.syntaxcoloring.DefaultAntlrTokenToAttributeIdMapper;

/**
 * 
 */
public class WMLAntlrTokenToAttributeIdMapper extends
    DefaultAntlrTokenToAttributeIdMapper
{
    @Override
    protected String calculateId( String tokenName, int tokenType )
    {
        if( tokenName.equals( "'+'" ) || tokenName.equals( "'['" )
            || tokenName.equals( "'[/'" ) || tokenName.equals( "']'" ) ) //$NON-NLS-1$ //$NON-NLS-2$
        {
            return WMLHighlightingConfiguration.RULE_WML_TAG;
        }
        if( tokenName.equals( "'~'" ) || tokenName.equals( "'{'" )
            || tokenName.equals( "'}'" ) ) //$NON-NLS-1$
        {
            return WMLHighlightingConfiguration.RULE_WML_MACRO_CALL;
        }
        return super.calculateId( tokenName, tokenType );
    }
}

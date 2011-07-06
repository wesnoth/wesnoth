/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.ArrayList;
import java.util.List;

import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTag;

public class WMLGrammarUtils
{
    public static List<WMLKey> getTagKeys( WMLTag tag )
    {
        List<WMLKey> result = new ArrayList<WMLKey>();
        for ( WMLExpression expression : tag.getExpressions( ) ) {
            if ( expression instanceof WMLKey )
                result.add( ( WMLKey ) expression );
        }

        return result;
    }

    public static List<WMLTag> getTagTags( WMLTag tag )
    {
        List<WMLTag> result = new ArrayList<WMLTag>();
        for ( WMLExpression expression : tag.getExpressions( ) ) {
            if ( expression instanceof WMLTag )
                result.add( ( WMLTag ) expression );
        }

        return result;
    }
}

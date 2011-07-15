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

import org.eclipse.emf.common.util.EList;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.INode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLMacroCallParameter;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.impl.WMLKeyValueImpl;

public class WMLGrammarUtils
{
    /**
     * Returns the list of child keys for this tag
     * @param tag The tag to process
     * @return A list of {@link WMLKey}
     */
    public static List<WMLKey> getTagKeys( WMLTag tag )
    {
        List<WMLKey> result = new ArrayList<WMLKey>();
        for ( WMLExpression expression : tag.getExpressions( ) ) {
            if ( expression instanceof WMLKey )
                result.add( ( WMLKey ) expression );
        }

        return result;
    }

    /**
     * Returns the list of child tags for this tag
     * @param tag The tag to process
     * @return A list of {@link WMLTag}
     */
    public static List<WMLTag> getTagTags( WMLTag tag )
    {
        List<WMLTag> result = new ArrayList<WMLTag>();
        for ( WMLExpression expression : tag.getExpressions( ) ) {
            if ( expression instanceof WMLTag )
                result.add( ( WMLTag ) expression );
        }

        return result;
    }

    /**
     * Returns the key value from the list as a string value
     * @param values The list of values of the key
     * @return A string representation of the key's value
     */
    public static String getKeyValue ( EList<WMLKeyValue> values )
    {
        StringBuilder result = new StringBuilder( );

        for ( WMLKeyValue value : values ) {
            if  ( value.getClass( ).equals( WMLKeyValueImpl.class ) ) {
                result.append( toStringWMLKeyValue( value ) );
            } else if ( value instanceof WMLMacroCall ) {
                result.append( toStringWMLMacroCall( (WMLMacroCall) value ) );
            }
        }

        return result.toString( );
    }

    /**
     * Returns the string representation of the specified WMLKeyValue
     * @param keyValue The key value instance
     * @return A string representation
     */
    public static String toStringWMLKeyValue( WMLKeyValue keyValue )
    {
        ICompositeNode node = NodeModelUtils.getNode( keyValue );
        if ( node == null )
            return "";

        StringBuilder result = new StringBuilder( );

        for ( INode tmpNode : node.getChildren( ) ) {
            for ( ILeafNode leafNode : tmpNode.getLeafNodes( ) ) {
                if ( leafNode.getLength( ) == 0 )
                    continue;

                result.append( leafNode.getText( ) );
            }
        }
        return result.toString( );
    }

    /**
     * Returns the string representation of the specified WMLMacroCall
     * @param macro The macro call instance
     * @return A string representation
     */
    public static String toStringWMLMacroCall( WMLMacroCall macro )
    {
        StringBuilder result = new StringBuilder( );
        result.append( "{" );
        result.append( macro.getPoint( ) );
        result.append( macro.getRelative( ) );

        result.append( macro.getName( ) );

        for ( WMLMacroCallParameter param : macro.getParameters( ) ) {
            if ( param instanceof WMLMacroCall )
                result.append( toStringWMLMacroCall( ( WMLMacroCall ) param ) );
            else {
                //
            }
        }

        result.append( "}" );
        return result.toString( );
    }
}

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
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.EObjectAtOffsetHelper;
import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;
import org.wesnoth.wml.WMLTag;

public class WMLUtils
{
    private static EObjectAtOffsetHelper eObjectAtOffsetHelper_;

    public static EObjectAtOffsetHelper EObjectUtils(){
        if ( eObjectAtOffsetHelper_ == null ) {
            eObjectAtOffsetHelper_ = new EObjectAtOffsetHelper( );
        }

        return eObjectAtOffsetHelper_;
    }

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
            result.append( toString( value ) );
        }

        return result.toString( );
    }

    /**
     * Returns the string representation of the specified WML object
     * with the preceeding space/new lines cleaned
     * @param object A WML EObject
     * @return A string representation
     */
    public static String toString( EObject object )
    {
        return toCleanedUpText( object );
    }

    private static String toCleanedUpText( EObject obj )
    {
        return NodeModelUtils.getNode( obj ).getText( )
                .replaceFirst( "(\\n|\\r| )+", "" );
    }
}

/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import org.eclipse.emf.common.util.EList;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.EObjectAtOffsetHelper;
import org.eclipse.xtext.resource.XtextResource;

import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;
import org.wesnoth.wml.WMLTag;

public class WMLUtils
{
    private static EObjectAtOffsetHelper eObjectAtOffsetHelper_;

    public static EObjectAtOffsetHelper EObjectUtils( )
    {
        if( eObjectAtOffsetHelper_ == null ) {
            eObjectAtOffsetHelper_ = new EObjectAtOffsetHelper( );
        }

        return eObjectAtOffsetHelper_;
    }

    public static EObject resolveElementAt( XtextResource resource, int offset )
    {
        try {
            return EObjectUtils( ).resolveElementAt( resource, offset );
        } catch( NullPointerException e ) {
            // in xtext 2.0, there is a bug, that if the file
            // contains errors, this method may throw a null pointer exception
            return null;
        }
    }

    /**
     * Returns the child key of the specified tag by its name.
     * 
     * @param tag
     *            The tag to search into
     * @param name
     *            The name of the key to search for
     * @return Returns the found key or null if non existing
     */
    public static WMLKey getKeyByName( WMLTag tag, String name )
    {
        for( WMLKey key: tag.getWMLKeys( ) ) {
            if( key.getName( ).equals( name ) )
                return key;
        }

        return null;
    }

    /**
     * Returns the child tag of the specified tag by its name.
     * 
     * @param tag
     *            The tag to search into
     * @param name
     *            The name of the tag to search for
     * @return Returns the found tag or null if non existing
     */
    public static WMLTag getTagByName( WMLTag tag, String name )
    {
        for( WMLTag subTag: tag.getWMLTags( ) ) {
            if( subTag.asWMLTag( ).getName( ).equals( name ) )
                return subTag;
        }

        return null;
    }

    /**
     * Returns the key value from the list as a string value
     * 
     * @param values
     *            The list of values of the key
     * @return A string representation of the key's value
     */
    public static String getKeyValue( EList< WMLKeyValue > values )
    {
        StringBuilder result = new StringBuilder( );

        for( WMLKeyValue value: values ) {
            result.append( toString( value ) );
        }

        return result.toString( );
    }

    /**
     * Returns a WML string representation of the specified tag
     * 
     * @param tag
     *            The tag to get the WML String representation for
     * @return The string representation
     */
    public static String toWMLString( WMLTag tag )
    {
        return toWMLString( tag, "" );
    }

    private static String toWMLString( WMLTag tag, String indent )
    {
        StringBuilder res = new StringBuilder( );

        res.append( indent + "[" + tag.getName( ) );
        if( ! tag.get_InhertedTagName( ).isEmpty( ) )
            res.append( ":" + tag.get_InhertedTagName( ) );
        res.append( "]\n" ); //$NON-NLS-1$ //$NON-NLS-2$

        for( WMLExpression expression: tag.getExpressions( ) ) {
            if( expression.isWMLKey( ) )
                res.append( indent + "\t"
                        + toWMLString( expression.asWMLKey( ) ) );
            else if( expression.isWMLTag( ) )
                res.append( indent + "\t"
                        + toWMLString( expression.asWMLTag( ) ) );
        }

        res.append( indent + "[/" + tag.getEndName( ) + "]\n" );
        return res.toString( );
    }

    /**
     * Returns a WML string representation of the specified key
     * 
     * @param tag
     *            The key to get the WML String representation for
     * @return The string representation
     */
    public static String toWMLString( WMLKey key )
    {
        return key.getName( ) + "=" + getKeyValue( key.getValues( ) );
    }

    /**
     * Returns the string representation of the specified WML object
     * with the preceeding space/new lines cleaned
     * 
     * @param object
     *            A WML EObject
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

/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * Represents a WML Variable
 */
public class WMLVariable implements Serializable
{
    private static final long serialVersionUID = 5293113569770337870L;

    private String            name_;
    private boolean           isArray_;

    private List< Scope >     scopes_;

    /**
     * Creates a new {@link WMLVariable} with the specified name
     * 
     * @param name
     *        The name of the variable
     */
    public WMLVariable( String name )
    {
        name_ = name;
        scopes_ = new ArrayList< Scope >( );
    }

    /**
     * @return The name of this {@link WMLVariable}
     */
    public String getName( )
    {
        return name_;
    }

    /**
     * @return True if the variable is an array, false otherwise
     */
    public boolean isArray( )
    {
        return isArray_;
    }

    /**
     * Sets whether this variable is an array or not
     * 
     * @param isArray
     *        True if the variable is an array, false otherwise
     */
    public void setIsArray( boolean isArray )
    {
        isArray_ = isArray;
    }

    /**
     * @return A list of {@link Scope} where this variable is visible
     */
    public List< Scope > getScopes( )
    {
        return scopes_;
    }

    @Override
    public String toString( )
    {
        StringBuilder res = new StringBuilder( );

        res.append( "Variable - Name: " + name_ );
        if( ! scopes_.isEmpty( ) ) {
            res.append( "; Scopes: " );

            for( Scope scope: scopes_ ) {
                res.append( scope );
            }
        }

        return res.toString( );
    }

    /**
     * Represents a scope of the WMLVariable
     */
    public static class Scope implements Serializable
    {
        private static final long serialVersionUID = - 1919240125062707719L;

        /**
         * The index of the start defined file
         */
        public int                StartIndex;
        /**
         * The offset in the start file
         */
        public int                StartOffset;

        /**
         * The index of the end undefined file
         */
        public int                EndIndex;
        /**
         * The offset in the end file
         */
        public int                EndOffset;

        /**
         * Creates a new scope
         * 
         * @param startIndex
         *        The start index of the scope
         * @param startOffset
         *        The offset in the file represented by the index
         */
        public Scope( int startIndex, int startOffset )
        {
            StartIndex = startIndex;
            StartOffset = startOffset;
            EndIndex = EndOffset = Integer.MAX_VALUE;
        }

        @Override
        public String toString( )
        {
            return "( " + StartIndex + ":" + StartOffset + " -> " + EndIndex
                + ":" + EndOffset + " )";
        }

        /**
         * Returns true if the specified index and offset lie withing
         * this scope
         * 
         * @param index
         *        The index of the file
         * @param offset
         *        The offset in the file
         * @return True of false
         */
        public boolean contains( int index, int offset )
        {
            return( ( StartIndex == index && EndIndex == index
                && StartOffset <= index && index <= EndOffset )
                || ( StartIndex == index && EndIndex != index && offset > StartOffset )
                || ( EndIndex == index && StartIndex != index && offset < EndOffset ) || ( StartIndex < index && index < EndIndex ) );
        }
    }
}

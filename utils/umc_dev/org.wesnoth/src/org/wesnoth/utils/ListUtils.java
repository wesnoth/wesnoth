/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.List;

/**
 * Utility class for lists
 */
public class ListUtils
{
    /**
     * Concatenates the list of Objects using the provided separator
     * 
     * @param list
     *        the list to concatenate
     * @param separator
     *        the separator used to concatenate the elements of the list
     * @return A string with the string representation of that objects
     */
    public static String concatenateList( List< ? extends Object > list,
        String separator )
    {
        if( list == null || list.isEmpty( ) ) {
            return ""; //$NON-NLS-1$
        }

        StringBuilder result = new StringBuilder( );
        for( int i = 0; i < list.size( ) - 1; i++ ) {
            result.append( list.get( i ) + separator );
        }
        result.append( list.get( list.size( ) - 1 ) );

        return result.toString( );
    }
}

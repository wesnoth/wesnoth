/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

public class Pair< T, K >
{
    public T First;
    public K Second;

    public Pair( T first, K second )
    {
        First = first;
        Second = second;
    }

    /**
     * Creates a new Pair
     * 
     * @param first
     *        The first item
     * @param second
     *        The second item
     * @return A new pair
     */
    public static < U, V > Pair< U, V > create( U first, V second )
    {
        return new Pair< U, V >( first, second );
    }

    @Override
    public String toString( )
    {
        return "( " + First.toString( ) + "; " + Second.toString( ) + " )";
    }
}

/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

/**
 * Simple mutable tuple that holds 2 object
 * 
 * @param <T>
 *        First object's Type
 * @param <K>
 *        Second object's Type
 */
public class Pair< T, K >
{
    /**
     * The first stored object
     */
    public T First;
    /**
     * The second stored object
     */
    public K Second;

    /**
     * Creates a new pair
     * 
     * @param first
     *        The first object
     * @param second
     *        The second object
     */
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

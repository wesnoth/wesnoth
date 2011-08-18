/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml;

import org.wesnoth.wml.impl.WmlFactory2Impl;

/**
 * Creates new WML Grammar Objects
 */
public interface WmlFactory2 extends WmlFactory
{
    /**
     * The singleton instance of the factory
     */
    WmlFactory2 eINSTANCE = new WmlFactory2Impl( );

    /**
     * Creates a new WML Tag
     * 
     * @param name
     *        The name of the tag
     * @return A new {@link WMLTag}
     */
    WMLTag createWMLTag( String name );

    /**
     * Creates a new WML Tag
     * 
     * @param name
     *        The name of the tag
     * @param extendedName
     *        The extended tag name
     * @return A new {@link WMLTag}
     */
    WMLTag createWMLTag( String name, String extendedName );

    /**
     * Creates a new WML Tag
     * 
     * @param name
     *        The name of the tag
     * @param extendedName
     *        The extended tag name
     * @param cardinality
     *        The cardinality of this tag
     * @return A new {@link WMLTag}
     */
    WMLTag createWMLTag( String name, String extendedName, char cardinality );

    /**
     * Creates a new WML Key
     * 
     * @param name
     *        The name of the key
     * @param dataType
     *        The type of the key
     * @return A new {@link WMLKey}
     */
    WMLKey createWMLKey( String name, String dataType );

    /**
     * Creates a new WML Key
     * 
     * @param name
     *        The name of the tag
     * @param dataType
     *        The type of the key
     * @param cardinality
     *        The cardinality of the key
     * @param translatable
     *        True if the key is translatable, false otherwise
     * @return A new {@link WMLKey}
     */
    WMLKey createWMLKey( String name, String dataType, char cardinality,
        boolean translatable );
}

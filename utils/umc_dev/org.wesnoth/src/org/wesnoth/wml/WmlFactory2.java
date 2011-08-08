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

public interface WmlFactory2 extends WmlFactory
{
    WmlFactory2 eINSTANCE = new WmlFactory2Impl( );

    WMLTag createWMLTag( String name );

    WMLTag createWMLTag( String name, String extendedName );

    WMLTag createWMLTag( String name, String extendedName, char cardinality );

    WMLKey createWMLKey( String name, String dataType );

    WMLKey createWMLKey( String name, String dataType, char cardinality,
            boolean translatable );
}

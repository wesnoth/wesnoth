/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml.impl;

import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLValue;
import org.wesnoth.wml.WmlFactory2;

/**
 * Implementation of the {@link WmlFactory2}
 */
public class WmlFactory2Impl extends WmlFactoryImpl implements WmlFactory2
{
    @Override
    public WMLTag createWMLTag( String name )
    {
        return createWMLTag( name, "" );
    }

    @Override
    public WMLTag createWMLTag( String name, String inhertedName )
    {
        return createWMLTag( name, inhertedName, '*' );
    }

    @Override
    public WMLTag createWMLTag( String name, String inhertedName,
        char cardinality )
    {
        WMLTag tag = createWMLTag( );

        tag.setName( name );
        tag.setEndName( name );

        tag.set_Cardinality( cardinality );
        tag.set_InhertedTagName( inhertedName );

        return tag;
    }

    @Override
    public WMLKey createWMLKey( String name, String dataType )
    {
        return createWMLKey( name, dataType, '*', false );
    }

    @Override
    public WMLKey createWMLKey( String name, String dataType, char cardinality,
        boolean translatable )
    {
        WMLKey key = createWMLKey( );

        key.setName( name );

        key.set_DataType( dataType );

        if( dataType.startsWith( "enum" ) ) {
            // add the enums values
            String[] res = dataType.substring( 4 ).split( "," );
            for( String string: res ) {
                if( string.length( ) == 0 ) {
                    continue;
                }

                WMLValue value = createWMLValue( );
                value.setValue( string );
                key.getValues( ).add( value );
            }
            key.set_Enum( true );
        }

        key.set_Cardinality( cardinality );
        key.set_Translatable( translatable );

        return key;
    }
}

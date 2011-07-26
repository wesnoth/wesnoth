/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml.core;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.wesnoth.Logger;
import org.wesnoth.schema.Tag;
import org.wesnoth.utils.StringUtils;

/**
 * This is a simple Lua parser that returns the found interesting tokens
 * from lua code
 */
public class SimpleLuaParser
{
    private List<Tag> tags_;
    private Reader reader_;

    private final static String TAG_REGEX = "wml_actions\\..+\\( *cfg *\\)";
    private final static String ATTRIBUTE_REGEX = "cfg\\.[a-zA-Z0-9_]*";
    private final static String ATTRIBUTE_CHILD_REGEX = "get_child\\(cfg, *\"[^\"]+\"\\)";

    public SimpleLuaParser( String contents )
    {
        tags_ = new ArrayList<Tag> ( );
        reader_ = new StringReader( contents == null ? "" : contents );
    }

    /**
     * Parses the lua code and gathers the list of tags
     */
    public void parse()
    {
        BufferedReader reader = new BufferedReader( reader_ );
        String line = null;

        Tag currentTag = null;

        try
        {
            while( ( line = reader.readLine( ) ) != null ) {
                List<String> tagTokens = StringUtils.getGroups( TAG_REGEX, line );

                // we handle just on tag per line
                if ( !tagTokens.isEmpty( ) ) {
                    String token = tagTokens.get( 0 );
                    // parse the tag name
                    String tagName = token.substring(
                            token.indexOf( '.' ) + 1,
                            token.lastIndexOf( '(' ) );

                    currentTag = new Tag( tagName, '*' );
                    tags_.add( currentTag );
                }

                // parse the attributes
                if ( currentTag != null ) {
                    List<String> attributeTokens = StringUtils.getGroups( ATTRIBUTE_REGEX, line );
                    for ( String token : attributeTokens ) {
                        String attributeName = token.substring(
                                token.indexOf( '.' ) + 1 );

                        currentTag.addKey( attributeName, "string", '*', true );
                    }

                    List<String> childTokens = StringUtils.getGroups( ATTRIBUTE_CHILD_REGEX, line );
                    for ( String token : childTokens ) {
                        String childName = token.substring(
                                token.indexOf( '"' ) + 1,
                                token.lastIndexOf( '"' ) );

                        currentTag.addKey( childName, "string", '*', true );
                    }
                }
            }
        }
        catch ( IOException e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Returns the parsed tags from the lua code
     * @return A list with Tags
     */
    public List<Tag> getTags()
    {
        return tags_;
    }
}

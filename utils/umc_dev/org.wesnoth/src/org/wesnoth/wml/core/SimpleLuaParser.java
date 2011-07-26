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

    private String TAG_REGEX = "wml_actions\\..+\\( *cfg *\\)";

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

        try
        {
            while( ( line = reader.readLine( ) ) != null ) {
                List<String> tokens = StringUtils.getGroups( TAG_REGEX, line );

                for ( String token : tokens ) {
                    // parse the tag name
                    String tagName = token.substring(
                            token.indexOf( '.' ) + 1,
                            token.length( ) - 1 );

                    tags_.add( new Tag( tagName, '*' ) );
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

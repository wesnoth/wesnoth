/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.wesnoth.Logger;
import org.wesnoth.utils.StringUtils;

/**
 * This is a simple Lua parser that returns the interesting tokens found
 * from lua code
 */
public class SimpleLuaParser
{
    private Map< String, WMLTag > tags_;
    private Reader                reader_;
    private String                location_;
    private int                   lineOffset_;

    private final static String   TAG_REGEX             = "wml_actions\\..+\\( *cfg *\\)";
    private final static String   ATTRIBUTE_REGEX       = "cfg\\.[a-zA-Z0-9_]*";
    private final static String   ATTRIBUTE_CHILD_REGEX = "get_child\\(cfg, *\"[^\"]+\"\\)";

    /**
     * Creates a new {@link SimpleLuaParser}
     * 
     * @param location
     *        The path location of the Lua code
     * @param contents
     *        The Lua code to parse
     */
    public SimpleLuaParser( String location, int lineOffset, String contents )
    {
        tags_ = new HashMap< String, WMLTag >( );
        reader_ = new StringReader( contents == null ? "": contents );

        location_ = location;
        lineOffset_ = lineOffset;
    }

    /**
     * Parses the lua code and gathers the list of tags
     */
    public void parse( )
    {
        BufferedReader reader = new BufferedReader( reader_ );
        String line = null;

        int lineCount = lineOffset_;

        WMLTag currentTag = null;

        try {
            while( ( line = reader.readLine( ) ) != null ) {
                List< String > tagTokens = StringUtils.getGroups( TAG_REGEX,
                    line );

                // we handle just on tag per line
                if( ! tagTokens.isEmpty( ) ) {
                    String token = tagTokens.get( 0 );
                    // parse the tag name
                    String tagName = token.substring( token.indexOf( '.' ) + 1,
                        token.lastIndexOf( '(' ) );

                    currentTag = WmlFactory2.eINSTANCE.createWMLTag( tagName );
                    currentTag.set_LuaBased( true );

                    if( location_ != null ) {
                        currentTag.set_DefinitionLocation( location_ );
                        currentTag.set_DefinitionOffset( lineCount );
                    }

                    tags_.put( tagName, currentTag );
                }

                // parse the attributes
                if( currentTag != null ) {
                    List< String > attributeTokens = StringUtils.getGroups(
                        ATTRIBUTE_REGEX, line );
                    for( String token: attributeTokens ) {
                        String attributeName = token.substring( token
                            .indexOf( '.' ) + 1 );

                        currentTag.getExpressions( ).add(
                            WmlFactory2.eINSTANCE.createWMLKey(
                                attributeName, "string", '1', false ) );
                    }

                    List< String > childTokens = StringUtils.getGroups(
                        ATTRIBUTE_CHILD_REGEX, line );
                    for( String token: childTokens ) {
                        String childName = token.substring(
                            token.indexOf( '"' ) + 1,
                            token.lastIndexOf( '"' ) );

                        currentTag.getExpressions( ).add(
                            WmlFactory2.eINSTANCE.createWMLKey( childName,
                                "string", '1', false ) );
                    }
                }

                ++lineCount;
            }
        } catch( IOException e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Returns the parsed tags from the lua code
     * 
     * @return A map with Tags
     */
    public Map< String, WMLTag > getTags( )
    {
        return tags_;
    }
}

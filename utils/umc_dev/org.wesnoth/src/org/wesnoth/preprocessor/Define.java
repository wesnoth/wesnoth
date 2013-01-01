/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preprocessor;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.eclipse.xtext.parser.IParser;

import org.wesnoth.Logger;
import org.wesnoth.WMLStandaloneSetup;
import org.wesnoth.wml.SimpleWMLParser;

/**
 * This represents a WML preprocessor define:
 * [preproc_define]
 * name = ""
 * value = ""
 * textdomain = ""
 * linenum = ""
 * location = ""
 * [/preproc_define]
 */
public class Define
{
    private String         name_;
    private String         value_;
    private String         textdomain_;
    private int            lineNum_;
    private String         location_;
    private List< String > args_;

    /**
     * Creates a new define with the specified details
     * 
     * @param name
     *        The name of the define
     * @param value
     *        The definition content of the define
     * @param textdomain
     *        The textdomain the define is belonging to
     * @param linenum
     *        The line number where the define is defined
     * @param location
     *        The file where the define is defined
     * @param args
     *        The arguments for the define
     */
    public Define( String name, String value, String textdomain, int linenum,
        String location, List< String > args )
    {
        name_ = name;
        value_ = value;
        textdomain_ = textdomain;
        lineNum_ = linenum;
        location_ = location;
        args_ = args;

        // ensure no NullPointerException exists
        if( args_ == null ) {
            args_ = new ArrayList< String >( );
        }
    }

    /**
     * Returns the line number where the define is defined
     * 
     * @return An integer representing the line number
     */
    public int getLineNum( )
    {
        return lineNum_;
    }

    /**
     * Returns the location name where the define is defined
     * 
     * @return A string representing the location
     */
    public String getLocation( )
    {
        return location_;
    }

    /**
     * Returns the name of the define
     * 
     * @return A string instance.
     */
    public String getName( )
    {
        return name_;
    }

    /**
     * Returns the textdomain where the define is define
     * 
     * @return A string instance
     */
    public String getTextdomain( )
    {
        return textdomain_;
    }

    /**
     * Returns the definition of the define
     * 
     * @return A string instance
     */
    public String getValue( )
    {
        return value_;
    }

    /**
     * Gets the arguments of this macro.
     * 
     * @return A {@link List} of strings.
     */
    public List< String > getArguments( )
    {
        return args_;
    }

    /**
     * Returns a string containing the current define, formatted
     * 
     * @return The string value of the define
     */
    @Override
    public String toString( )
    {
        StringBuilder res = new StringBuilder( );
        res.append( "[preproc_define]" ); //$NON-NLS-1$
        res.append( "\tname=\"" + name_ + "\"" );
        res.append( "\tvalue=\"" + value_ + "\"" );
        res.append( "\ttextdomain=\"" + textdomain_ + "\"" );
        res.append( "\tlinenum=\"" + lineNum_ + "\"" );
        res.append( "\tlocation=\"" + location_ + "\"" );
        res.append( "[/preproc_define]" ); //$NON-NLS-1$
        return res.toString( );
    }

    /**
     * Reads the defines from the specified file
     * 
     * @param file
     *        The file to read the defines from
     * @return Returns a map of defines
     */
    public static Map< String, Define > readDefines( String file )
    {
        try {
            SimpleWMLParser parser = new SimpleWMLParser( new File( file ),
                WMLStandaloneSetup.getInjector( ).getInstance( IParser.class ) );

            parser.parse( );
            return parser.getDefines( );

        } catch( FileNotFoundException e ) {
            Logger.getInstance( ).logException( e );
            return new HashMap< String, Define >( 0 );
        }
    }
}

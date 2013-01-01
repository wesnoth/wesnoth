/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

import org.wesnoth.Logger;

/**
 * Utilities class that handles Strings
 */
public class StringUtils
{
    /**
     * Returns true if the 'target' starts with 'sequence'.
     * The tabs or spaces in front are skipped when checking for the 'sequence'
     * 
     * @param target
     *        The target to check.
     * @param sequence
     *        The sequence to search for.
     * @return True if the string starts with, false otherwise.
     */
    public static boolean startsWith( String target, String sequence )
    {
        if( target == null || sequence == null ) {
            return false;
        }

        Pattern pattern = Pattern
            .compile( "[\t ]*" + Pattern.quote( sequence ) ); //$NON-NLS-1$
        Matcher matcher = pattern.matcher( target );
        return( matcher.find( ) && matcher.start( ) == 0 );
    }

    /**
     * Returns the number of 'character' characters in the target string
     * 
     * @param target
     *        The String to perform the count on.
     * @param character
     *        The character to count.
     * @return An integer representing the number of apparitions.
     */
    public static int countOf( String target, char character )
    {
        if( target == null ) {
            return 0;
        }

        if( target.indexOf( character ) == - 1 ) {
            return 0;
        }

        int cnt = 0;
        String tmpString = target;
        while( tmpString.contains( new String( new char[] { character } ) ) ) {
            ++cnt;
            tmpString = tmpString
                .substring( tmpString.indexOf( character ) + 1 );
        }
        return cnt;
    }

    /**
     * Removes all consecutive aparitions of a character in the specified string
     * so that only one appearance remains in each past duplications of that
     * string
     * 
     * @param target
     *        the string to process
     * @param character
     *        the character to remove
     * @param removeTrailing
     *        removes or not the trailing 'character' characters
     * @param removePreceding
     *        removes or not the preceding 'character' characters
     * @return The processed string.
     */
    public static String removeIncorrectCharacters( String target,
        char character, boolean removeTrailing, boolean removePreceding )
    {
        if( target == null ) {
            return ""; //$NON-NLS-1$
        }

        StringBuilder resString = new StringBuilder( );

        for( int i = 0; i < target.length( ); i++ ) {
            // pass over successive repetitions:
            // abbbac will become: abac
            if( i > 0 && ( target.charAt( i ) == target.charAt( i - 1 ) ) ) {
                continue;
            }

            if( target.charAt( i ) == character
                && ( ( removeTrailing && i == target.length( ) ) || ( removePreceding && i == 0 ) ) ) {
                continue;
            }

            resString.append( target.charAt( i ) );
        }
        return resString.toString( );
    }

    /**
     * Gets an array of the lines (without line breakes) which compund the
     * string
     * 
     * @param string
     *        The string
     * @return A {@link String} array with the lines.
     */
    public static String[] getLines( String string )
    {
        if( string == null ) {
            return new String[0];
        }
        return string.split( "\\r?\\n" ); //$NON-NLS-1$
    }

    /**
     * Removes all trailing path separators from the string
     * 
     * @param string
     *        The string to trim
     * @return A new string that doesn't have any trailling separators
     */
    public static String trimEndPathSeparators( String string )
    {
        if( isNullOrEmpty( string ) ) {
            return ""; //$NON-NLS-1$
        }

        while( string.charAt( string.length( ) - 1 ) == '/'
            || string.charAt( string.length( ) - 1 ) == '\\' ) {
            string = string.substring( 0, string.length( ) - 1 );
        }
        return string;
    }

    /**
     * Trims any quotes " of the specified string
     * 
     * @param string
     *        The string to trim
     * @return A new string with the quotes trimmed
     */
    public static String trimQuotes( String string )
    {
        if( isNullOrEmpty( string ) ) {
            return "";
        }

        while( string.charAt( 0 ) == '"' ) {
            string = string.substring( 1 );
        }

        int strSize = string.length( ) - 1;
        while( strSize >= 0 && string.charAt( strSize ) == '"' ) {
            string = string.substring( 0, strSize );
            --strSize;
        }

        return string;
    }

    /**
     * Normalizes the path given by the string, removing repeated separators
     * and replacing them by '|'
     * 
     * @param path
     *        the string that represents the path to be normalized
     * @return The normalized string.
     */
    public static String normalizePath( String path )
    {
        if( path == null ) {
            return ""; //$NON-NLS-1$
        }

        String str = StringUtils.trimEndPathSeparators( path );
        StringBuilder sb = new StringBuilder( str.length( ) );

        // normalize strings - remove repeating separators and replace them by |
        for( int i = 0, sw = 0; i < str.length( ); i++ ) {
            if( str.charAt( i ) == '/' || str.charAt( i ) == '\\' ) {
                if( sw == 0 ) // we haven't added already the | token
                {
                    sb.append( '|' );
                    sw = 1;
                }
                continue;
            }
            sb.append( str.charAt( i ) );
            sw = 0; // reset the switch when meeting non-separators
        }
        return sb.toString( );
    }

    /**
     * Replaces the source with the target in the specified string,
     * adding the existing indentation (if any) to all lines (if any) in the
     * target
     * 
     * @param string
     *        The string where to replace the source
     * @param source
     *        The source string to be replaced
     * @param target
     *        The target string to be replaced in the string
     * @return The processed string.
     */
    public static String replaceWithIndent( String string, String source,
        String target )
    {
        if( string == null ) {
            return ""; //$NON-NLS-1$
        }

        // get the current indentation
        Pattern pattern = Pattern.compile( "[\t ]*" ); //$NON-NLS-1$
        Matcher matcher = pattern.matcher( string );
        String indent = ""; //$NON-NLS-1$
        if( matcher.find( ) ) {
            int end = matcher.end( );
            indent = string.substring( matcher.start( ), end );
        }

        String[] tmpTarget = getLines( target );
        for( int i = 1; i < tmpTarget.length; i++ ) {
            tmpTarget[i] = indent + tmpTarget[i];
        }

        return string.replace( source,
            ListUtils.concatenateList( Arrays.asList( tmpTarget ), "\n" ) ); //$NON-NLS-1$
    }

    /**
     * Returns a sequence multiplied by the specified number of times
     * 
     * @param sequence
     *        The sequence to multiply
     * @param times
     *        The number of times to multiply
     * @return The new string.
     */
    public static String multiples( String sequence, int times )
    {
        if( sequence == null ) {
            return ""; //$NON-NLS-1$
        }

        StringBuilder res = new StringBuilder( sequence.length( ) * times );
        for( int i = 0; i < times; i++ ) {
            res.append( sequence );
        }
        return res.toString( );
    }

    /**
     * Returns true if the specified string is null or empty
     * 
     * @param target
     *        The string to check
     * @return True if the string is null or empty, false otherwise.
     */
    public static boolean isNullOrEmpty( String target )
    {
        return( target == null || target.isEmpty( ) );
    }

    /**
     * Returns the list of groups found by the specified regex
     * in the specified string.
     * The regex is applied case insensitive
     * 
     * @param regexStr
     *        The regex used to extract the groups
     * @param targetString
     *        The string on which to apply the regex
     * @return The list of strings which matched the regex
     */
    public static List< String > getGroups( String regexStr, String targetString )
    {
        List< String > groupList = new ArrayList< String >( );
        try {
            Pattern regex = Pattern
                .compile( regexStr, Pattern.CASE_INSENSITIVE );
            Matcher regexMatcher = regex.matcher( targetString );
            while( regexMatcher.find( ) ) {
                groupList.add( regexMatcher.group( ) );
            }
        } catch( PatternSyntaxException ex ) {
            Logger.getInstance( ).logException( ex );
        }
        return groupList;
    }
}

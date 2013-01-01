/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling.wmldoc;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyleRange;
import org.eclipse.swt.widgets.Display;

import org.wesnoth.preprocessor.Define;
import org.wesnoth.ui.Messages;


/**
 * Provides WMLDoc info for a macro
 */
public class WMLDocMacro implements IWMLDocProvider
{
    private Define             macro_;
    private String             title_;
    private String             contents_;
    private List< StyleRange > styleRanges_;
    private boolean            docGenerated_;

    /**
     * Creates a new {@link WMLDocMacro}
     * 
     * @param macro
     *        The associated {@link Define}
     */
    public WMLDocMacro( Define macro )
    {
        macro_ = macro;
        docGenerated_ = false;
    }

    /**
     * A method used for lazly generating the documentation
     */
    private void generateDoc( )
    {
        if( docGenerated_ ) {
            return;
        }

        styleRanges_ = new ArrayList< StyleRange >( );
        title_ = Messages.WMLDocMacro_0 + macro_.getName( );
        StringBuilder content = new StringBuilder( );

        content.append( Messages.WMLDocMacro_1 );
        addStyleRange( 0, content.length( ) - 1, SWT.BOLD );
        content.append( macro_.getValue( ) );

        content.append( '\n' );
        if( macro_.getArguments( ).isEmpty( ) == false ) {
            int len = content.length( ) - 1;
            content.append( Messages.WMLDocMacro_2 );

            addStyleRange( len, content.length( ) - len - 1, SWT.BOLD );

            len = content.length( ) - 1;
            for( String arg: macro_.getArguments( ) ) {
                content.append( '\t' + arg + '\n' );
            }
            addStyleRange( len, content.length( ) - len - 1, SWT.ITALIC );
        }

        contents_ = content.toString( );

        docGenerated_ = true;
    }

    /**
     * Adds a style range to current list
     * 
     * @param offset
     * @param length
     * @param style
     */
    private void addStyleRange( int offset, int length, int style )
    {
        styleRanges_.add( new StyleRange( offset, length, Display.getDefault( )
            .getSystemColor( SWT.COLOR_INFO_FOREGROUND ), Display
            .getDefault( ).getSystemColor( SWT.COLOR_INFO_BACKGROUND ),
            style ) );
    }

    @Override
    public String getTitle( )
    {
        generateDoc( );
        return title_;
    }

    @Override
    public String getContents( )
    {
        generateDoc( );
        return contents_;
    }

    @Override
    public StyleRange[] getStyleRanges( )
    {
        generateDoc( );
        return styleRanges_.toArray( new StyleRange[styleRanges_.size( )] );
    }

    /**
     * Gets the associated macro
     * 
     * @return Gets the associated macro
     */
    public Define getMacro( )
    {
        return macro_;
    }

    @Override
    public String getInfoText( )
    {
        if( macro_.getLocation( ) == null ) {
            return null;
        }
        return "Defined in: " + macro_.getLocation( ) + " : "
            + macro_.getLineNum( );
    }
}

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

import org.eclipse.core.resources.IFile;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyleRange;
import org.eclipse.swt.widgets.Display;

import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.ui.Messages;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WmlFactory2;

/**
 * Displays wml doc for a tag [tag] or [/tag]
 */
public class WMLDocTag implements IWMLDocProvider
{
    private WMLTag             tag_;
    private String             title_;
    private String             contents_;
    private List< StyleRange > styleRanges_;

    private boolean            docGenerated_;

    /**
     * Creates a new {@link WMLDocTag}
     * 
     * @param currentFile
     *        The file where to create the doc on
     * @param installName
     *        The installname used when creating the doc
     * @param name
     *        The name of the tag to present
     */
    public WMLDocTag( IFile currentFile, String installName, String name )
    {
        tag_ = SchemaParser.getInstance( installName ).getTags( ).get( name );

        // try to get it from the Project Cache ( lua parsed ones )
        if( tag_ == null ) {
            tag_ = ProjectUtils.getCacheForProject( currentFile.getProject( ) )
                .getWMLTags( ).get( name );
        }

        // Create a default one
        if( tag_ == null ) {
            tag_ = WmlFactory2.eINSTANCE.createWMLTag( name );
        }

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

        title_ = Messages.WMLDocTag_0 + tag_.getName( ) + "':"; //$NON-NLS-1$

        contents_ = "No information";

        if( ! tag_.get_Description( ).isEmpty( ) ) {
            StringBuilder content = new StringBuilder( );
            content.append( Messages.WMLDocTag_1 );
            addStyleRange( 0, content.length( ) - 1, SWT.BOLD );
            content.append( tag_.get_Description( ) );
            contents_ = content.toString( );
        }

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
    public String getInfoText( )
    {
        String infoText = "";
        if( tag_.is_LuaBased( ) ) {
            infoText += "[Lua tag] ";
        }

        if( ! tag_.get_DefinitionLocation( ).isEmpty( ) ) {
            infoText += "Defined in: " + tag_.get_DefinitionLocation( );
            infoText += " : " + tag_.get_DefinitionOffset( );
        }
        return infoText.isEmpty( ) ? null: infoText;
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
}

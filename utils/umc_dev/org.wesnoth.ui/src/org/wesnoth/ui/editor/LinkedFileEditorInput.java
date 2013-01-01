/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.editor;

import java.util.Locale;

import org.eclipse.core.resources.IFile;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.ide.FileStoreEditorInput;
import org.eclipse.ui.part.FileEditorInput;

/**
 * We use an auxiliary class just to do a 'better' equals check
 */
public class LinkedFileEditorInput extends FileEditorInput
{
    /**
     * Creates a new {@link LinkedFileEditorInput}
     * 
     * @param file
     *        The file attached to this editor input
     */
    public LinkedFileEditorInput( IFile file )
    {
        super( file );
    }

    @Override
    public boolean equals( Object obj )
    {
        if( this == obj ) {
            return true;
        }
        String targetUri = ""; //$NON-NLS-1$
        if( ! ( obj instanceof IFileEditorInput ) ) {
            if( obj instanceof FileStoreEditorInput ) {
                FileStoreEditorInput other = ( FileStoreEditorInput ) obj;
                targetUri = other.getURI( ).toString( );
            }
        }
        else {
            IFileEditorInput other = ( IFileEditorInput ) obj;
            targetUri = other.getFile( ).getLocationURI( ).toString( );
        }
        if( targetUri.isEmpty( ) ) {
            return false;
        }
        return getFile( ).getLocationURI( ).toString( )
            .toLowerCase( Locale.ENGLISH )
            .equals( targetUri.toLowerCase( Locale.ENGLISH ) );
    }
}

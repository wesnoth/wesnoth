/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.folding;

import com.google.inject.Inject;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.resource.ILocationInFileProvider;
import org.eclipse.xtext.ui.editor.folding.DefaultFoldingRegionProvider;
import org.eclipse.xtext.ui.editor.folding.IFoldingRegionAcceptor;
import org.eclipse.xtext.util.ITextRegion;

import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLMacroDefine;
import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLTextdomain;

/**
 * Helper for providing correct folding regions
 */
public class WMLFoldingRegionProvider extends DefaultFoldingRegionProvider
{
    @Inject
    private ILocationInFileProvider locationInFileProvider;

    @Override
    protected boolean isHandled( EObject eObject )
    {
        if( eObject instanceof WMLTextdomain ) {
            return false;
        }
        else if( eObject instanceof WMLKey ) {
            WMLKey key = ( WMLKey ) eObject;
            if( ! key.getEol( ).isEmpty( ) ) {
                return false;
            }
        }

        return super.isHandled( eObject );
    }

    @Override
    protected void computeObjectFolding( EObject eObject,
        IFoldingRegionAcceptor< ITextRegion > foldingRegionAcceptor )
    {
        // copied from "DefaultFoldingRegionProvider
        ITextRegion region = locationInFileProvider.getFullTextRegion( eObject );
        if( region != null ) {
            ITextRegion significant = locationInFileProvider
                .getSignificantTextRegion( eObject );
            if( significant == null ) {
                throw new NullPointerException(
                    "significant region may not be null" );
            }
            int offset = region.getOffset( );
            int length = region.getLength( );

            String endName = "";
            if( eObject instanceof WMLPreprocIF ) {
                endName = ( ( WMLPreprocIF ) eObject ).getEndName( );
            }
            else if( eObject instanceof WMLMacroDefine ) {
                endName = ( ( WMLMacroDefine ) eObject ).getEndName( );
            }

            if( endName.endsWith( "\r\n" ) ) {
                length -= 2;
            }
            else if( endName.endsWith( "\n" ) ) {
                --length;
            }

            foldingRegionAcceptor.accept( offset, length, significant );
        }
    }
}

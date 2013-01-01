/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import java.util.Iterator;

import org.eclipse.emf.common.notify.Adapter;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.IRegion;
import org.eclipse.jface.text.source.DefaultCharacterPairMatcher;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.model.XtextDocument;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.eclipse.xtext.util.concurrent.IUnitOfWork;

import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLTag;

/**
 * The class uses the character pair matcher to implement the
 * start/end tag matching
 */
public class WMLCharacterPairMatcher extends DefaultCharacterPairMatcher
{
    private WMLTag currentTag_;

    private int    matchCnt = 0;

    /**
     * Creates a new {@link WMLCharacterPairMatcher}
     * 
     * @param chars
     *        The chars to match
     */
    public WMLCharacterPairMatcher( char[] chars )
    {
        super( chars );
        currentTag_ = null;
    }

    @Override
    public IRegion match( IDocument doc, final int offset )
    {
        ++matchCnt;
        IRegion region = super.match( doc, offset );

        if( region == null && doc instanceof XtextDocument
            && doc.getLength( ) > 0 ) {
            if( matchCnt == 2 ) {
                matchCnt = 0;
            }
            else {
                ( ( XtextDocument ) doc )
                    .readOnly( new IUnitOfWork< Boolean, XtextResource >( ) {

                        @Override
                        public Boolean exec( XtextResource state )
                            throws Exception
                        {
                            computeMatchingRegion( state, offset );
                            return true;
                        }
                    } );

                // refresh the highlighting
                WMLEditor currentEditor = ( WMLEditor ) EditorUtils
                    .getActiveXtextEditor( );
                if( currentEditor != null
                    && currentEditor.getHighlightingHelper( ) != null
                    && currentEditor.getHighlightingHelper( )
                        .getReconciler( ) != null ) {
                    currentEditor.getHighlightingHelper( ).getReconciler( )
                        .refresh( );
                }
            }
        }

        return region;
    }

    private synchronized void computeMatchingRegion( XtextResource state,
        int offset )
    {
        EObject object = WMLUtils.resolveElementAt( state, offset );

        // do nothing if we clicked the same tag
        if( currentTag_ == object ) {
            return;
        }

        if( object instanceof WMLTag ) {
            WMLTag tag = ( WMLTag ) object;

            currentTag_ = tag;
            for( Adapter adapter: state.eAdapters( ) ) {
                if( adapter instanceof WMLSyntaxColoringAdapter ) {

                    ( ( WMLSyntaxColoringAdapter ) adapter ).TargetEObject = object;
                    return; // done here
                }
            }

            state.eAdapters( )
                .add( new WMLSyntaxColoringAdapter(
                    WMLHighlightingConfiguration.RULE_MATCH_TAG, object ) );
        }
        else {
            // nothing new selected, just remove current adapter
            Iterator< Adapter > itor = state.eAdapters( ).iterator( );
            while( itor.hasNext( ) ) {
                if( itor.next( ) instanceof WMLSyntaxColoringAdapter ) {
                    itor.remove( );
                    break;
                }
            }

            currentTag_ = null;
        }
    }
}

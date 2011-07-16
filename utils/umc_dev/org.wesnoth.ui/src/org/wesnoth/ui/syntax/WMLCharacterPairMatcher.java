/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
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
import org.eclipse.xtext.util.concurrent.IUnitOfWork;
import org.wesnoth.ui.WMLSyntaxColoringAdapter;
import org.wesnoth.ui.WMLUtil;
import org.wesnoth.wml.WMLTag;

public class WMLCharacterPairMatcher extends DefaultCharacterPairMatcher
{
    private WMLSyntaxColoringAdapter currentAdapter_;
    private WMLTag currentTag_;

    private int matchCnt = 0;

    public WMLCharacterPairMatcher( char[] chars )
    {
        super( chars );
        currentAdapter_ = null;
        currentTag_ = null;
    }

    @Override
    public IRegion match( IDocument doc, final int offset )
    {
        ++ matchCnt;
        IRegion region = super.match( doc, offset );

        if ( region == null && doc instanceof XtextDocument ) {
            if ( matchCnt == 2 ) {
                matchCnt = 0;
            } else {
                ( ( XtextDocument ) doc ).modify( new IUnitOfWork<IRegion, XtextResource>(){

                    @Override
                    public IRegion exec( XtextResource state ) throws Exception
                    {
                        return computeMatchingRegion( state, offset );
                    }

                });
            }
        }

        return region;
    }

    public IRegion computeMatchingRegion(XtextResource state, int offset)
    {
        EObject object = WMLUtil.EObjectUtils( ).resolveElementAt( state, offset );

        // do nothing if we clicked the same tag
        if ( currentTag_ == object )
            return null;

        // remove current colored tag ( if any )
        if ( currentTag_ != null ) {
            synchronized ( currentTag_ ) {

                Iterator<Adapter> itor = currentTag_.eAdapters( ).iterator( );
                while ( itor.hasNext( ) ) {
                    if ( itor.next( ) instanceof WMLSyntaxColoringAdapter ) {
                        itor.remove( );
                    }
                }

                currentAdapter_ = null;
                currentTag_ = null;
            }
        }

        if ( object instanceof WMLTag ) {
            WMLTag tag = ( WMLTag ) object;

            currentAdapter_ = new WMLSyntaxColoringAdapter( WMLHighlightingConfiguration.RULE_MATCH_TAG, true );
            currentTag_ = tag;
            tag.eAdapters( ).add( currentAdapter_ );
        }

        return null;
    }
}

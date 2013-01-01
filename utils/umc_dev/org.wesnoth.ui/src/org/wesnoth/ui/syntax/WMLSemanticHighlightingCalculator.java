/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.eclipse.emf.common.notify.Adapter;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.INode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightedPositionAcceptor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.ISemanticHighlightingCalculator;

import org.wesnoth.utils.Pair;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLMacroDefine;
import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLTextdomain;
import org.wesnoth.wml.WmlPackage;

/**
 * Highlights the WML code semantically
 */
public class WMLSemanticHighlightingCalculator implements
    ISemanticHighlightingCalculator
{
    @Override
    public void provideHighlightingFor( XtextResource resource,
        IHighlightedPositionAcceptor acceptor )
    {
        if( resource == null ) {
            return;
        }

        Iterator< EObject > iter = EcoreUtil.getAllContents( resource, true );
        while( iter.hasNext( ) ) {
            EObject current = iter.next( );

            List< Pair< INode, String > > toColor = new ArrayList< Pair< INode, String > >( );

            if( current instanceof WMLTag ) {
                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_TAG ) );

                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_TAG__END_NAME ),
                    WMLHighlightingConfiguration.RULE_WML_TAG ) );
            }
            else if( current instanceof WMLKey ) {
                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_KEY ) );
            }
            else if( current instanceof WMLMacroCall ) {
                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_MACRO_CALL ) );
            }
            else if( current instanceof WMLTextdomain ) {
                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_TEXTDOMAIN ) );
            }
            else if( current instanceof WMLPreprocIF ) {
                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_IF ) );

                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_IF ) );
            }
            else if( current instanceof WMLMacroDefine ) {
                toColor.add( Pair.create(
                    getFirstFeatureNode( current,
                        WmlPackage.Literals.WML_EXPRESSION__NAME ),
                    WMLHighlightingConfiguration.RULE_WML_MACRO_DEFINE ) );

                toColor.add( Pair
                    .create(
                        getFirstFeatureNode(
                            current,
                            WmlPackage.Literals.WML_MACRO_DEFINE__END_NAME ),
                        WMLHighlightingConfiguration.RULE_WML_MACRO_DEFINE ) );
            }

            // check if we have any information specific information for
            // highlighting
            for( Adapter adapter: resource.eAdapters( ) ) {
                if( adapter instanceof WMLSyntaxColoringAdapter ) {
                    WMLSyntaxColoringAdapter wmlAdapter = ( WMLSyntaxColoringAdapter ) adapter;

                    if( wmlAdapter.TargetEObject == current ) {

                        for( Pair< INode, String > pair: toColor ) {
                            pair.Second = wmlAdapter.ColorId;
                        }

                    }

                    break;
                }
            }

            for( Pair< INode, String > pair: toColor ) {
                highlightNode( pair.First, pair.Second, acceptor );
            }
        }
    }

    /**
     * Copied from org.eclipse.xtext.xtext.ui.editor.syntaxcoloring.
     * SemanticHighlightingCalculator
     * 
     * @param node
     *        The node to highlight
     * @param id
     *        The id of the coloring
     * @param acceptor
     *        The acceptor to add the node to
     */
    private void highlightNode( INode node, String id,
        IHighlightedPositionAcceptor acceptor )
    {
        if( node == null ) {
            return;
        }
        if( node instanceof ILeafNode ) {
            acceptor.addPosition( node.getOffset( ), node.getLength( ), id );
        }
        else {
            for( ILeafNode leaf: node.getLeafNodes( ) ) {
                if( ! leaf.isHidden( ) ) {
                    acceptor.addPosition( leaf.getOffset( ), leaf.getLength( ),
                        id );
                }
            }
        }
    }

    /**
     * Copied from org.eclipse.xtext.xtext.ui.editor.syntaxcoloring.
     * SemanticHighlightingCalculator
     * 
     * @param semantic
     *        The object to search in
     * @param feature
     *        The feature to search for
     * @return Null if there is no feature, or an {@link INode} node
     */
    public INode getFirstFeatureNode( EObject semantic,
        EStructuralFeature feature )
    {
        if( feature == null ) {
            return NodeModelUtils.findActualNodeFor( semantic );
        }
        List< INode > nodes = NodeModelUtils.findNodesForFeature( semantic,
            feature );
        if( ! nodes.isEmpty( ) ) {
            return nodes.get( 0 );
        }
        return null;
    }
}

/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.autoedit;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.DocumentCommand;
import org.eclipse.jface.text.IAutoEditStrategy;
import org.eclipse.jface.text.IDocument;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.eclipse.xtext.util.concurrent.IUnitOfWork;

import org.wesnoth.utils.StringUtils;
import org.wesnoth.wml.WMLTag;

/**
 * Strategy that tries to close the nearest unclosed tag
 */
public class ClosingEndTagAutoEditStrategy implements IAutoEditStrategy
{
    @Override
    public void customizeDocumentCommand( final IDocument document,
        final DocumentCommand command )
    {
        try {
            if( command.text.equals( "/" ) && document.get( command.offset - 1, 1 ).equals( "[" ) ) //$NON-NLS-1$ //$NON-NLS-2$
            {
                XtextEditor editor = EditorUtils.getActiveXtextEditor( );
                if( editor == null ) {
                    return;
                }

                editor.getDocument( ).readOnly(
                    new IUnitOfWork.Void< XtextResource >( ) {
                        @Override
                        public void process( XtextResource state )
                            throws Exception
                        {
                            ILeafNode currentNode = NodeModelUtils
                                .findLeafNodeAtOffset( state
                                    .getParseResult( )
                                    .getRootNode( ), command.offset );

                            if( currentNode == null ) {
                                return;
                            }

                            EObject semanticElement = currentNode
                                .getSemanticElement( );
                            if( semanticElement == null ) {
                                return;
                            }

                            String tagName = ""; //$NON-NLS-1$
                            EObject container = semanticElement
                                .eContainer( );
                            if( container instanceof WMLTag ) {
                                tagName = ( ( WMLTag ) container )
                                    .getName( );
                            }

                            if( ! StringUtils.isNullOrEmpty( tagName ) ) {
                                command.shiftsCaret = true;
                                command.text = ( "/" + tagName ); //$NON-NLS-1$
                            }
                        }
                    } );
            }
        } catch( BadLocationException e ) {
        }
    }
}

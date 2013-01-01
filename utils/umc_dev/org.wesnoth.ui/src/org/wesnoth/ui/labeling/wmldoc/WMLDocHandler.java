/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling.wmldoc;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.swt.graphics.Point;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.eclipse.xtext.util.concurrent.IUnitOfWork;

import org.wesnoth.Logger;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLTag;

/**
 * A handler that handles pressing F2 on a resource in the editor
 */
public class WMLDocHandler extends AbstractHandler
{
    @Override
    public Object execute( ExecutionEvent event ) throws ExecutionException
    {
        try {
            final XtextEditor editor = EditorUtils.getActiveXtextEditor( event );
            final IFile editedFile = WMLEditor.getEditorFile( editor );
            final String installName = WesnothInstallsUtils
                .getInstallNameForResource( editedFile );

            editor.getDocument( ).readOnly(
                new IUnitOfWork.Void< XtextResource >( ) {
                    @Override
                    public void process( XtextResource resource )
                        throws Exception
                    {
                        ITextSelection selection = ( ITextSelection ) editor
                            .getSelectionProvider( ).getSelection( );
                        Point positionRelative = editor
                            .getInternalSourceViewer( )
                            .getTextWidget( )
                            .getLocationAtOffset( selection.getOffset( ) );
                        Point positionAbsolute = editor
                            .getInternalSourceViewer( ).getTextWidget( )
                            .toDisplay( positionRelative );
                        positionAbsolute.y += 20;

                        EObject grammarElement = WMLUtils.resolveElementAt(
                            resource, selection.getOffset( ) );
                        if( grammarElement == null ) {
                            return;
                        }

                        if( grammarElement instanceof WMLMacroCall ) {
                            WMLMacroCall macro = ( WMLMacroCall ) grammarElement;
                            Define define = ProjectUtils
                                .getCacheForProject(
                                    editedFile.getProject( ) )
                                .getDefines( ).get( macro.getName( ) );
                            if( define != null ) {
                                WMLDocInformationPresenter presenter = new WMLDocInformationPresenter(
                                    editor.getSite( ).getShell( ),
                                    new WMLDocMacro( define ),
                                    positionAbsolute );
                                presenter.create( );
                                presenter.open( );
                            }
                        }
                        else if( grammarElement instanceof WMLTag ) {
                            WMLDocInformationPresenter presenter = new WMLDocInformationPresenter(
                                editor.getSite( ).getShell( ),
                                new WMLDocTag( editedFile, installName,
                                    ( ( WMLTag ) grammarElement )
                                        .getName( ) ),
                                positionAbsolute );
                            presenter.create( );
                            presenter.open( );
                        }
                    }
                } );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
        }
        return null;
    }
}

/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
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
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.swt.graphics.Point;
import org.eclipse.xtext.nodemodel.INode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.parser.IParseResult;
import org.eclipse.xtext.resource.EObjectAtOffsetHelper;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.eclipse.xtext.util.concurrent.IUnitOfWork;
import org.wesnoth.Logger;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.ui.WMLUtil;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLTag;

import com.google.inject.Inject;

/**
 * A handler that handles pressing F2 on a resource in the editor
 */
public class WMLDocHandler extends AbstractHandler
{
    @Inject
    private EObjectAtOffsetHelper eObjectAtOffsetHelper;

	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		try
		{
			final XtextEditor editor = EditorUtils.getActiveXtextEditor(event);
			final String installName =
			    WesnothInstallsUtils.getInstallNameForResource( WMLUtil.getActiveEditorFile( ) );

			editor.getDocument().readOnly(new IUnitOfWork.Void<XtextResource>()
			{
				private WMLDocInformationPresenter presenter_;
				@Override
				public void process(XtextResource resource) throws Exception
				{
					ITextSelection selection = (ITextSelection) editor.getSelectionProvider().getSelection();
                	Point positionRelative = editor.getInternalSourceViewer().getTextWidget().getLocationAtOffset(selection.getOffset());
                	Point positionAbsolute = editor.getInternalSourceViewer().getTextWidget().toDisplay(positionRelative);
                	positionAbsolute.y +=20;

                	IParseResult parseResult = resource.getParseResult();
                	//TODO: check this
                	INode abstractNode = NodeModelUtils.getNode( eObjectAtOffsetHelper.resolveElementAt( resource, selection.getOffset( ) ) );
                	if (abstractNode == null )
                		return;

                	EObject grammarElement = abstractNode.getGrammarElement( );
                    if ( grammarElement instanceof WMLMacroCall )
                    {
                    	WMLMacroCall macro = (WMLMacroCall) grammarElement;
                    	Define define = ProjectUtils.getCacheForProject(
                    			WMLUtil.getActiveEditorFile().getProject())
                    			.getDefines().get(macro.getName());
                    	if (define != null)
                    	{
                    		if (presenter_ == null)
                    		{
                    			presenter_ = new WMLDocInformationPresenter(
                    					editor.getSite().getShell(),
                    					new WMLDocMacro(define),
                    					positionAbsolute);
                    			presenter_.create();
                    		}
                    		presenter_.open();
                    	}
                    }
                    else if ( grammarElement instanceof WMLTag)
                    {
                    	if (presenter_ == null)
                    	{
                    		presenter_ = new WMLDocInformationPresenter(
                    					editor.getSite().getShell(),
                    					new WMLDocTag( installName, ( ( WMLTag ) grammarElement ).getName() ),
                    					positionAbsolute);
                    		presenter_.create();
                    	}
                    	presenter_.open();
                    }
                }
			});
		}
		catch (Exception e) {
			Logger.getInstance().logException(e);
		}
		return null;
	}
}

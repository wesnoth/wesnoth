/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
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
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.swt.graphics.Point;
import org.eclipse.xtext.parser.IParseResult;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.ParseTreeUtil;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.eclipse.xtext.util.concurrent.IUnitOfWork;
import org.wesnoth.ui.WMLUtil;
import org.wesnoth.wML.WMLMacroCall;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.ProjectUtils;

/**
 * A handler that handles pressing F2 on a resource in the editor
 */
public class WMLDocHandler extends AbstractHandler
{
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		try
		{
			final XtextEditor editor = EditorUtils.getActiveXtextEditor(event);
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
                	AbstractNode abstractNode = ParseTreeUtil.getCurrentOrPrecedingNodeByOffset(parseResult.getRootNode(), selection.getOffset());
                	if (abstractNode == null || abstractNode.eContainer() == null)
                		return;

                	CompositeNode container = (CompositeNode)abstractNode.eContainer();
                    if (container.getElement() instanceof WMLMacroCall) {
                    	WMLMacroCall macro = (WMLMacroCall)container.getElement();

                    	if (ProjectUtils.getCacheForProject(WMLUtil.getActiveEditorFile().getProject())
                    			.getDefines().containsKey(macro.getName()))
                    	{
                    		if (presenter_ == null) {
                    			presenter_ = new WMLDocInformationPresenter(editor.getSite().getShell(),
                    					macro.getName(),
                    					"Macro: " + "", "",
                    					positionAbsolute);
                    			presenter_.create();
                    		}
                    		presenter_.open();
                    	}
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

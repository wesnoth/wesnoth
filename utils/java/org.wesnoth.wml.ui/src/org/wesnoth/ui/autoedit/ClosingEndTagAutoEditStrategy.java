/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.autoedit;

import org.eclipse.emf.common.util.EList;
import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.DocumentCommand;
import org.eclipse.jface.text.IAutoEditStrategy;
import org.eclipse.jface.text.IDocument;
import org.eclipse.xtext.parser.IParseResult;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.eclipse.xtext.util.concurrent.IUnitOfWork;

/**
 * Strategy that tries to close the nearest unclosed tag
 */
public class ClosingEndTagAutoEditStrategy implements IAutoEditStrategy
{
	public void customizeDocumentCommand(final IDocument document,
			final DocumentCommand command)
	{
		try
		{
			if (command.text.equals("/") && document.get(command.offset-1, 1).equals("["))
			{
				XtextEditor editor = EditorUtils.getActiveXtextEditor();
				if (editor == null)
					return;
				editor.getDocument().readOnly(new IUnitOfWork.Void<XtextResource>(){
					@Override
					public void process(XtextResource state) throws Exception
					{
						IParseResult parseResult = state.getParseResult();
						if(parseResult == null)
							return;
						CompositeNode rootNode = parseResult.getRootNode();
						LeafNode node = (LeafNode) NodeUtil.findLeafNodeAtOffset(rootNode, command.offset);

						String tagName = "";
						EList<AbstractNode> children = node.getParent().getParent().getChildren();
						for(int i = 0 ;i < children.size(); ++i)
						{
							if ((children.get(i) instanceof LeafNode) == false)
								continue;
							// we need one more child
							if (i+1 >= children.size())
								continue;
							if (((LeafNode)children.get(i)).getText().equals("["))
							{
								if (children.get(i+1) instanceof LeafNode)
								{
									// in case we have [+
									if (((LeafNode)children.get(i+1)).getText().equals("+"))
									{
										if (i+2 >= children.size() ||
											(children.get(i+2) instanceof LeafNode) == false)
											continue;
										tagName = ((LeafNode)children.get(i+2)).getText();
									}
									else
									{
										tagName = ((LeafNode)children.get(i+1)).getText();
									}
								}
							}
						}
						if (tagName.isEmpty() == false)
						{
							command.shiftsCaret = true;
							command.text = ("/" + tagName + "]");
						}
					}
				});
			}
		} catch (BadLocationException e) {
		}
	}
}

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.contentassist;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.Assignment;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.ui.editor.contentassist.ContentAssistContext;
import org.eclipse.xtext.ui.editor.contentassist.ICompletionProposalAcceptor;

import wesnoth_eclipse_plugin.schema.SchemaParser;
import wesnoth_eclipse_plugin.schema.Tag;

public class WMLProposalProvider extends AbstractWMLProposalProvider
{
	public final static boolean DEBUG_AUTOCOMPLETE = true;

	public WMLProposalProvider()
	{
		super();
		// load the schema so we know what to suggest for autocomplete
		SchemaParser.getInstance().parseSchema(false);
	}

	@Override
	public void completeWMLTag_Name(EObject model, Assignment assignment,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.completeWMLTag_Name(model, assignment, context, acceptor);
		dbg("completing wmltagname");
		if (context.getCurrentNode().eContainer() != null &&
			context.getCurrentNode().eContainer() instanceof AbstractNode)
		{
			AbstractNode node = (AbstractNode)context.getCurrentNode().eContainer();

			if (node.getParent().eContainer() != null) // we are not at the root
			{
				LeafNode parent = (LeafNode)NodeUtil.findLeafNodeAtOffset(node.getParent(),
						node.getParent().getOffset() + 2);
				dbg(parent.getText());
				Tag tagChildren = SchemaParser.getInstance().getTags().get(parent.getText());
				if (tagChildren != null)
				{
					for(Tag tag : tagChildren.getTagChildren())
					{
						acceptor.accept(createCompletionProposal(tag.getName(),
								context));
					}
				}
				else
					dbg("!!! no tag found with that name:" + parent.getText());
			}
			else // we are at the root
			{
				Tag rootTag = SchemaParser.getInstance().getTags().get("root");
				dbg("root node. adding tags: "+ rootTag.getTagChildren().size());
				for(Tag tag : rootTag.getTagChildren())
				{
					acceptor.accept(createCompletionProposal(tag.getName(), context));
				}
			}
		}
	}

	/**
	 * Method for debugging the auto completion
	 * @param str
	 */
	private void dbg(Object str)
	{
		if (!(DEBUG_AUTOCOMPLETE))
			return;
		System.out.println(str.toString());
	}
}

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.contentassist;

import java.util.Properties;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.contentassist.ICompletionProposal;
import org.eclipse.xtext.Assignment;
import org.eclipse.xtext.RuleCall;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.ui.editor.contentassist.ContentAssistContext;
import org.eclipse.xtext.ui.editor.contentassist.ICompletionProposalAcceptor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;
import org.wesnoth.ui.WMLUiModule;
import org.wesnoth.ui.labeling.WMLLabelProvider;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLTag;
import org.wesnoth.wML.impl.WMLFactoryImpl;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.schema.SchemaParser;
import wesnoth_eclipse_plugin.schema.Tag;
import wesnoth_eclipse_plugin.schema.TagKey;
import wesnoth_eclipse_plugin.utils.ProjectUtils;

public class WMLProposalProvider extends AbstractWMLProposalProvider
{
	public WMLProposalProvider()
	{
		super();
		// load the schema so we know what to suggest for autocomplete
		SchemaParser.getInstance().parseSchema(false);
	}

	@Override
	public void completeWMLKey_Name(EObject model, Assignment assignment,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.completeWMLKey_Name(model, assignment, context, acceptor);
		dbg("completing wmlkeyname");
		addKeyNameProposals(model, context, acceptor);
	}

	@Override
	public void complete_WMLKeyValue(EObject model, RuleCall ruleCall,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.complete_WMLKeyValue(model, ruleCall, context, acceptor);
		dbg("completing wmlkeyvalue");
		addKeyValueProposals(model, context, acceptor);
	}

	@Override
	public void complete_WMLTag(EObject model, RuleCall ruleCall,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.complete_WMLTag(model, ruleCall, context, acceptor);
		dbg("completing wmltag - rule");
		addTagProposals(model, true, context, acceptor);
	}

	@Override
	public void completeWMLTag_Name(EObject model, Assignment assignment,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.completeWMLTag_Name(model, assignment, context, acceptor);
		dbg("completing wmltagname");
		addTagProposals(model, false, context, acceptor);
	}

	/**
	 * Adss the wml key's names proposals
	 * @param model
	 * @param context
	 * @param acceptor
	 */
	private void addKeyValueProposals(EObject model,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		if (model != null && model instanceof WMLKey)
		{
			dbg(model);
			WMLKey key = (WMLKey)model;

			// handle the next_scenario
			if (key.getName().equals("next_scenario"))
			{
				IFile file = (IFile)EditorUtils.getActiveXtextEditor()
								.getEditorInput().getAdapter(IFile.class);
				if (file == null)
				{
					Logger.getInstance().logError("FATAL! file is null (and it shouldn't)");
					return;
				}
				Properties props = ProjectUtils.getPropertiesForProject(file.getProject());
				//TODO: dummy entry. remove when proper architecture is ready
				props.setProperty("scenarios", "01_scen1,02_scen2,");
				if (props.getProperty("scenarios") != null)
				{
					String[] scenarios = props.getProperty("scenarios").split(",");
					for(String scenarioId : scenarios)
					{
						if (scenarioId.isEmpty())
							continue;
						acceptor.accept(createCompletionProposal(scenarioId,
								scenarioId, WMLLabelProvider.getImageByName("scenario.png"),
								context));
					}
				}
			}
		}
	}

	/**
	 * Adss the wml key's names proposals
	 * @param model
	 * @param context
	 * @param acceptor
	 */
	private void addKeyNameProposals(EObject model,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		if (model instanceof WMLTag)
		{
			WMLTag tag = (WMLTag)model;
			if (SchemaParser.getInstance().	getTags().get(tag.getName()) != null)
			{
				for(TagKey key : SchemaParser.getInstance().
						getTags().get(tag.getName()).getKeyChildren())
				{
					acceptor.accept(createCompletionProposal(key.getName() + "=",
							key.getName(),
							getImage(WMLFactoryImpl.eINSTANCE.getWMLPackage().getWMLKey()),
							context));
				}
			}
		}
	}

	/**
	 * Adds the tag proposals
	 * @param model
	 * @param ruleProposal
	 * @param context
	 * @param acceptor
	 */
	private void addTagProposals(EObject model, boolean ruleProposal,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		if (checkContextNodeIsAbstractNode(context) &&
			((AbstractNode)context.getCurrentNode().eContainer()).getParent() != null &&
			((AbstractNode)context.getCurrentNode().eContainer()).getParent().eContainer() != null)
		{
			AbstractNode node = (AbstractNode)context.getCurrentNode().eContainer();

			LeafNode parent = (LeafNode)NodeUtil.findLeafNodeAtOffset(node.getParent(),
					node.getParent().getOffset() + 2);
			String parentIndent = ((LeafNode)NodeUtil.findLeafNodeAtOffset(node.getParent(),
					node.getOffset())).getText();

			// remove ugly new lines that break indentation
			parentIndent =  parentIndent.replace("\r", "").replace("\n", "");

			Tag tagChildren = SchemaParser.getInstance().getTags().get(parent.getText());
			if (tagChildren != null)
			{
				for(Tag tag : tagChildren.getTagChildren())
				{
					acceptor.accept(tagProposal(tag, parentIndent, ruleProposal, context));
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
				acceptor.accept(tagProposal(tag, "", ruleProposal, context));
			}
		}
	}

	private boolean checkContextNodeIsAbstractNode(ContentAssistContext context)
	{
		return (context.getCurrentNode().eContainer() != null &&
				context.getCurrentNode().eContainer() instanceof AbstractNode);
	}

	/**
	 * Returns the proposal for the specified tag, usign the specified indent
	 * @param tag The tag from which to construct the proposal
	 * @param indent The indent used to indent the tag and subsequent keys
	 * @param ruleProposal Whether this is a proposal for an entire rule or not
	 * @param context
	 * @return
	 */
	private ICompletionProposal tagProposal(Tag tag, String indent, boolean ruleProposal,
					ContentAssistContext context)
	{
//		dbg("indent:[" + indent +"]");
		StringBuilder proposal = new StringBuilder();
		if (ruleProposal)
			proposal.append("[");
		proposal.append(tag.getName());
		proposal.append("]\n");
		for(TagKey key : tag.getKeyChildren())
		{
			if (key.getCardinality() == '1')
				proposal.append(String.format("\t%s%s=\n",
						indent, key.getName()));
		}
		proposal.append(String.format("%s[/%s]",indent, tag.getName()));
		return createCompletionProposal(proposal.toString(), tag.getName(),
					getImage(WMLFactoryImpl.eINSTANCE.getWMLPackage().getWMLTag()),
					context);
	}

	private ICompletionProposal createCompletionProposal(String proposal, String displayString,
					ContentAssistContext context)
	{
		return createCompletionProposal(proposal, displayString, null, context);
	}

	/**
	 * Method for debugging the auto completion
	 * @param str
	 */
	private void dbg(Object str)
	{
		if (!(WMLUiModule.DEBUG))
			return;
		System.out.println(str.toString());
	}
}

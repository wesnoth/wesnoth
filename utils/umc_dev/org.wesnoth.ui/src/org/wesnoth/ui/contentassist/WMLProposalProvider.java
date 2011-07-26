/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.contentassist;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.contentassist.ICompletionProposal;
import org.eclipse.jface.viewers.StyledString;
import org.eclipse.swt.graphics.Image;
import org.eclipse.xtext.Assignment;
import org.eclipse.xtext.RuleCall;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.ui.editor.contentassist.ContentAssistContext;
import org.eclipse.xtext.ui.editor.contentassist.ICompletionProposalAcceptor;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.schema.Tag;
import org.wesnoth.schema.TagKey;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.ui.WMLUiModule;
import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.ui.labeling.WMLLabelProvider;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.core.WMLConfig;
import org.wesnoth.wml.core.WMLVariable;

import com.google.common.base.Function;
import com.google.common.base.Predicates;
import com.google.common.collect.Collections2;

public class WMLProposalProvider extends AbstractWMLProposalProvider
{
    protected SchemaParser schemaParser_;
    protected ProjectCache projectCache_;
    protected int dependencyIndex_;

    protected static final int KEY_VALUE_PRIORITY = 1700;
    protected static final int KEY_NAME_PRIORITY = 1500;
    protected static final int TAG_PRIORITY = 1000;
    protected static final int MACRO_CALL_PRIORITY = 100;

    private static Image MACRO_CALL_IMAGE = null;
    private static Image SCENARIO_VALUE_IMAGE = null;
    private static Image WML_KEY_IMAGE = null;
    private static Image WML_TAG_IMAGE = null;

	/**
	 * For priorities, see:
	 * {@link #KEY_NAME_PRIORITY}
	 * {@link #KEY_VALUE_PRIORITY}
	 * {@link #TAG_PRIORITY}
	 * {@link #MACRO_CALL_PRIORITY}
	 */
	public WMLProposalProvider()
	{
		super();

		MACRO_CALL_IMAGE = WMLLabelProvider.getImageByName("macrocall.png");
		SCENARIO_VALUE_IMAGE = WMLLabelProvider.getImageByName("scenario.png");
		WML_KEY_IMAGE = WMLLabelProvider.getImageByName("wmlkey.png");
		WML_TAG_IMAGE = WMLLabelProvider.getImageByName("wmltag.png");
	}

	/**
	 * Refreshes the current project cache/schema based
	 * on the file opened
	 */
	private void refresh()
	{
	    if ( projectCache_ != null )
	        return;

        IFile file = WMLEditor.getActiveEditorFile();
        projectCache_ = ProjectUtils.getCacheForProject( file.getProject( ) );

        // load the schema so we know what to suggest for autocomplete
        SchemaParser.reloadSchemas( false );
        schemaParser_ = SchemaParser.getInstance( WesnothInstallsUtils.getInstallNameForResource( file ) );

        dependencyIndex_ = ResourceUtils.getDependencyIndex( file );
	}

	@Override
	public void completeWMLKey_Name(EObject model, Assignment assignment,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.completeWMLKey_Name(model, assignment, context, acceptor);
		refresh( );
		dbg("completing wmlkeyname"); //$NON-NLS-1$
		addKeyNameProposals(model, context, acceptor);
	}

	@Override
	public void complete_WMLKeyValue(EObject model, RuleCall ruleCall,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.complete_WMLKeyValue(model, ruleCall, context, acceptor);
		refresh( );
		dbg("completing wmlkeyvalue - rule"); //$NON-NLS-1$
		addKeyValueProposals(model, context, acceptor);
	}

	@Override
	public void complete_WMLTag(EObject model, RuleCall ruleCall,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.complete_WMLTag(model, ruleCall, context, acceptor);
		refresh( );
		dbg("completing wmltag - rule"); //$NON-NLS-1$
		addTagProposals(model, true, context, acceptor);
	}

	@Override
	public void completeWMLTag_Name(EObject model, Assignment assignment,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.completeWMLTag_Name(model, assignment, context, acceptor);
		refresh( );
		dbg("completing wmltagname"); //$NON-NLS-1$
		addTagProposals(model, false, context, acceptor);
	}

	@Override
	public void completeWMLMacroCall_Name(EObject model, Assignment assignment,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.completeWMLMacroCall_Name(model, assignment, context, acceptor);
		refresh( );
		dbg("completing wmlmacrocallname"); //$NON-NLS-1$
		addMacroCallProposals(model, false, context, acceptor);
	}

	@Override
	public void complete_WMLMacroCall(EObject model, RuleCall ruleCall,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		super.complete_WMLMacroCall(model, ruleCall, context, acceptor);
		refresh( );
		dbg("completing wmlmacrocall - rule"); //$NON-NLS-1$
		addMacroCallProposals(model, true, context, acceptor);
	}

	private void addMacroCallProposals(EObject model, boolean ruleProposal,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		for(Entry<String, Define> define : projectCache_.getDefines().entrySet())
		{
			StringBuilder proposal = new StringBuilder(10);
			if (ruleProposal == true)
				proposal.append("{"); //$NON-NLS-1$
			proposal.append(define.getKey());

			for(String arg : define.getValue().getArguments())
				proposal.append(" " + arg); //$NON-NLS-1$
			proposal.append("}"); //$NON-NLS-1$

			acceptor.accept(createCompletionProposal(proposal.toString(), define.getKey(),
					MACRO_CALL_IMAGE, context, MACRO_CALL_PRIORITY)); //$NON-NLS-1$
		}
	}

	private void addKeyValueProposals(EObject model,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		if ( model == null || !( model instanceof WMLKey ) )
		    return;
		dbg(model);
		WMLKey key = (WMLKey)model;
		String keyName = key.getName( );

		// handle the next_scenario and first_scenario
		if ( keyName.equals("next_scenario") || //$NON-NLS-1$
			 keyName.equals("first_scenario")) //$NON-NLS-1$
		{
			for(WMLConfig config : projectCache_.getWMLConfigs().values())
			{
				if (StringUtils.isNullOrEmpty( config.ScenarioId ))
					continue;
				acceptor.accept(createCompletionProposal(config.ScenarioId,
						config.ScenarioId, SCENARIO_VALUE_IMAGE, //$NON-NLS-1$
						context, KEY_VALUE_PRIORITY));
			}
		}
		else if (model.eContainer() != null && model.eContainer() instanceof WMLTag)
		{
		    WMLTag parent = (WMLTag) model.eContainer();
		    String tagName = parent.getName();
		    Tag tag = schemaParser_.getTags().get( tagName );
		    if (tag != null)
		    {
		        TagKey tagKey = tag.getChildKey( keyName );
		        if (tagKey.isEnum())
		        {
		            String[] values = tagKey.getValue().split(","); //$NON-NLS-1$
		            for(String val : values)
		            {
		                acceptor.accept(createCompletionProposal(val, context, KEY_VALUE_PRIORITY));
		            }
		        }
		    }

		    if ( ( tagName.equals( "event" ) || tagName.equals( "fire_event" ) )
		            && keyName.equals( "name" ) ) {
		        // add events
		        List<String> events = TemplateProvider.getInstance( ).getCAC( "events" );

		        for ( String event : events ) {
		            acceptor.accept( createCompletionProposal( event, context ) );
		        }
		    } else {
		        // add variables
		        List<String> variables = new ArrayList<String>();
                variables.addAll( TemplateProvider.getInstance( ).getCAC( "variables" ) );

		        // filter variables by index
		        Collection<String> projectVariables = Collections2.transform(
		                projectCache_.getVariables( ).values( ),
                        new Function<WMLVariable, String> () {

                    @Override
                    public String apply( WMLVariable from )
                    {
//                        if ( from.getScopeStartIndex( ) <= dependencyIndex_ &&
//                             dependencyIndex_ <= from.getScopeEndIndex( ) )
//                            return from.getName( );
                        return null;
                    }
                } );

		        variables.addAll( Collections2.filter( projectVariables,
	                Predicates.notNull( ) ) );

		        for ( String variable : variables ) {
		            acceptor.accept( createCompletionProposal( "$" + variable, context ) );
		        }
		    }
		}
	}

	private void addKeyNameProposals(EObject model,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		WMLTag tag = null;
		if (model instanceof WMLTag)
			tag = (WMLTag)model;
		else if (model.eContainer() instanceof WMLTag)
			tag = (WMLTag)model.eContainer();

		if (tag != null)
		{
		    Tag schemaTag = schemaParser_.getTags().get(tag.getName());
			if ( schemaTag != null)
			{
				boolean found = false;
				for(TagKey key : schemaTag.getKeyChildren())
				{
					// skip forbidden keys
					if (key.isForbidden())
						continue;

					found = false;
					// check only non-repeatable keys
					if (key.isRepeatable() == false)
					{
						// don't suggest already completed keys
						for( WMLKey eKey: WMLUtils.getTagKeys( tag ) )
							if (eKey.getName().equals(key.getName())) {
								found = true;
								break;
							}
					}

					if (found == false)
						acceptor.accept(createCompletionProposal(key.getName() + "=", //$NON-NLS-1$
							key.getName(), WML_KEY_IMAGE, context, KEY_NAME_PRIORITY));
				}
			}
		}
	}

	private void addTagProposals(EObject model, boolean ruleProposal,
			ContentAssistContext context, ICompletionProposalAcceptor acceptor)
	{
		WMLTag parentTag = null;
		if (model instanceof WMLTag)
			parentTag = (WMLTag)model;
		else if (model.eContainer() instanceof WMLTag)
			parentTag = (WMLTag)model.eContainer();

		if (parentTag != null)
		{
		    ICompositeNode node = NodeModelUtils.getNode(model);

			String parentIndent = ""; //$NON-NLS-1$
			if (context.getCurrentNode().getOffset() > 0)
				parentIndent = NodeModelUtils.findLeafNodeAtOffset(node,
						context.getCurrentNode().getOffset() -
						// if we have a non-rule proposal, subtract 1
						(ruleProposal ? 0 : 1) ).getText();

			// remove ugly new lines that break indentation
			parentIndent =  parentIndent.replace("\r", "").replace("\n", ""); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$

			Tag tagChildren = schemaParser_.getTags().get(parentTag.getName());
			if (tagChildren != null)
			{
				boolean found = false;
				for(Tag tag : tagChildren.getTagChildren())
				{
					// skip forbidden tags
					if (tag.isForbidden())
						continue;

					found = false;

					// check only non-repeatable tags
					if (tag.isRepeatable() == false)
					{
						for( WMLTag wmlTag : WMLUtils.getTagTags( parentTag ) )
							if (wmlTag.getName().equals(tag.getName()))
							{
								found = true;
								break;
							}
					}

					if (found == false)
						acceptor.accept(createTagProposal(tag, parentIndent,
								ruleProposal, context));
				}
			}
			else
				dbg("!!! no tag found with that name:" + parentTag.getName()); //$NON-NLS-1$
		}
		else // we are at the root
		{
			Tag rootTag = schemaParser_.getTags().get("root"); //$NON-NLS-1$
			dbg("root node. adding tags: "+ rootTag.getTagChildren().size()); //$NON-NLS-1$
			for(Tag tag : rootTag.getTagChildren())
			{
				acceptor.accept(createTagProposal(tag, "", ruleProposal, context)); //$NON-NLS-1$
			}
		}
	}

	/**
	 * Returns the proposal for the specified tag, usign the specified indent
	 * @param tag The tag from which to construct the proposal
	 * @param indent The indent used to indent the tag and subsequent keys
	 * @param ruleProposal Whether this is a proposal for an entire rule or not
	 * @param context
	 * @return
	 */
	private ICompletionProposal createTagProposal(Tag tag, String indent, boolean ruleProposal,
					ContentAssistContext context)
	{
		StringBuilder proposal = new StringBuilder();
		if (ruleProposal)
			proposal.append("["); //$NON-NLS-1$
		proposal.append(tag.getName());
		proposal.append("\n"); //$NON-NLS-1$
		for(TagKey key : tag.getKeyChildren())
		{
			if (key.isRequired())
				proposal.append(String.format("\t%s%s=\n", //$NON-NLS-1$
						indent, key.getName()));
		}
		proposal.append(String.format("%s[/%s",indent, tag.getName())); //$NON-NLS-1$
		return createCompletionProposal(proposal.toString(), tag.getName(),
		        WML_TAG_IMAGE, context, TAG_PRIORITY);
	}

	private ICompletionProposal createCompletionProposal(String proposal,
			ContentAssistContext context, int priority)
	{
		return createCompletionProposal(proposal, null, null, priority,
				context.getPrefix(), context);
	}

	public ICompletionProposal createCompletionProposal(String proposal, String displayString, Image image,
			ContentAssistContext contentAssistContext, int priority)
	{
		return createCompletionProposal(proposal, new StyledString(displayString),
					image, priority, contentAssistContext.getPrefix(), contentAssistContext);
	}

	/**
	 * Method for debugging the auto completion
	 * @param str
	 */
	@SuppressWarnings( "unused" )
    private void dbg(Object str)
	{
		if (!(WMLUiModule.DEBUG))
			return;
		System.out.println(str.toString());
	}
}

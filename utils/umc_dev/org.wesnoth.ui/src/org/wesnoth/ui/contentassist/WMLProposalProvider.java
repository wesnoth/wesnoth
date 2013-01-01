/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.contentassist;

import com.google.common.base.Function;
import com.google.common.base.Predicates;
import com.google.common.collect.Collections2;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.contentassist.ICompletionProposal;
import org.eclipse.jface.viewers.ILabelProvider;
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
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLConfig;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLVariable;
import org.wesnoth.wml.WMLVariable.Scope;

/**
 * Proposal provider for WML content assist
 */
public class WMLProposalProvider extends AbstractWMLProposalProvider
{
    protected SchemaParser     schemaParser_;
    protected ProjectCache     projectCache_;
    protected int              dependencyIndex_;

    protected static final int KEY_VALUE_PRIORITY   = 1700;
    protected static final int KEY_NAME_PRIORITY    = 1500;
    protected static final int TAG_PRIORITY         = 1000;
    protected static final int MACRO_CALL_PRIORITY  = 100;

    private static boolean     IMAGES_INITED        = false;
    private static Image       MACRO_CALL_IMAGE     = null;
    private static Image       SCENARIO_VALUE_IMAGE = null;
    private static Image       WML_KEY_IMAGE        = null;
    private static Image       WML_TAG_IMAGE        = null;

    /**
     * For priorities, see: {@link #KEY_NAME_PRIORITY}
     * {@link #KEY_VALUE_PRIORITY} {@link #TAG_PRIORITY}
     * {@link #MACRO_CALL_PRIORITY}
     */
    public WMLProposalProvider( )
    {
        super( );
    }

    private void initImages( )
    {
        if( IMAGES_INITED ) {
            return;
        }

        ILabelProvider labelProvider = getLabelProvider( );

        MACRO_CALL_IMAGE = labelProvider.getImage( "macrocall.png" );
        SCENARIO_VALUE_IMAGE = labelProvider.getImage( "scenario.png" );
        WML_KEY_IMAGE = labelProvider.getImage( "wmlkey.png" );
        WML_TAG_IMAGE = labelProvider.getImage( "wmltag.png" );
    }

    /**
     * Initializes the proposal provider with all needed values
     */
    private void initProposalProvider( )
    {
        if( projectCache_ != null ) {
            return;
        }

        IFile file = WMLEditor.getActiveEditorFile( );
        projectCache_ = ProjectUtils.getCacheForProject( file.getProject( ) );

        // load the schema so we know what to suggest for autocomplete
        schemaParser_ = SchemaParser.getInstance( WesnothInstallsUtils
            .getInstallNameForResource( file ) );

        dependencyIndex_ = ResourceUtils.getDependencyIndex( file );

        initImages( );
    }

    @Override
    public void createProposals( ContentAssistContext context,
        ICompletionProposalAcceptor acceptor )
    {
        initProposalProvider( );

        super.createProposals( context, acceptor );
    }

    @Override
    public void completeWMLKey_Name( EObject model, Assignment assignment,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        super.completeWMLKey_Name( model, assignment, context, acceptor );

        addKeyNameProposals( model, context, acceptor );
    }

    @Override
    public void complete_WMLKeyValue( EObject model, RuleCall ruleCall,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        super.complete_WMLKeyValue( model, ruleCall, context, acceptor );

        addKeyValueProposals( model, context, acceptor );
    }

    @Override
    public void complete_WMLTag( EObject model, RuleCall ruleCall,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        super.complete_WMLTag( model, ruleCall, context, acceptor );

        addTagProposals( model, true, context, acceptor );
    }

    @Override
    public void completeWMLTag_Name( EObject model, Assignment assignment,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        super.completeWMLTag_Name( model, assignment, context, acceptor );

        addTagProposals( model, false, context, acceptor );
    }

    @Override
    public void completeWMLMacroCall_Name( EObject model,
        Assignment assignment, ContentAssistContext context,
        ICompletionProposalAcceptor acceptor )
    {
        super.completeWMLMacroCall_Name( model, assignment, context, acceptor );

        addMacroCallProposals( model, false, context, acceptor );
    }

    @Override
    public void complete_WMLMacroCall( EObject model, RuleCall ruleCall,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        super.complete_WMLMacroCall( model, ruleCall, context, acceptor );

        addMacroCallProposals( model, true, context, acceptor );
    }

    private void addMacroCallProposals( EObject model, boolean ruleProposal,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        for( Entry< String, Define > define: projectCache_.getDefines( )
            .entrySet( ) ) {
            StringBuilder proposal = new StringBuilder( 10 );
            if( ruleProposal == true ) {
                proposal.append( "{" ); //$NON-NLS-1$
            }
            proposal.append( define.getKey( ) );

            for( String arg: define.getValue( ).getArguments( ) ) {
                proposal.append( " " + arg ); //$NON-NLS-1$
            }
            proposal.append( "}" ); //$NON-NLS-1$

            acceptor.accept( createCompletionProposal( proposal.toString( ),
                define.getKey( ), MACRO_CALL_IMAGE, context,
                MACRO_CALL_PRIORITY ) );
        }
    }

    private void addKeyValueProposals( EObject model,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        if( model == null || ! ( model instanceof WMLKey ) ) {
            return;
        }
        WMLKey key = ( WMLKey ) model;
        String keyName = key.getName( );

        // handle the next_scenario and first_scenario
        if( keyName.equals( "next_scenario" ) || //$NON-NLS-1$
            keyName.equals( "first_scenario" ) ) //$NON-NLS-1$
        {
            for( WMLConfig config: projectCache_.getWMLConfigs( ).values( ) ) {
                if( StringUtils.isNullOrEmpty( config.ScenarioId ) ) {
                    continue;
                }
                acceptor.accept( createCompletionProposal( config.ScenarioId,
                    config.ScenarioId, SCENARIO_VALUE_IMAGE, context,
                    KEY_VALUE_PRIORITY ) );
            }
        }
        else if( model.eContainer( ) != null
            && model.eContainer( ) instanceof WMLTag ) {
            WMLTag parent = ( WMLTag ) model.eContainer( );
            String tagName = parent.getName( );
            WMLTag tag = schemaParser_.getTags( ).get( tagName );
            if( tag != null ) {
                WMLKey tagKey = WMLUtils.getKeyByName( tag, keyName );
                if( tagKey != null && tagKey.is_Enum( ) ) {
                    for( WMLKeyValue val: tagKey.getValues( ) ) {
                        acceptor.accept( createCompletionProposal(
                            val.toString( ), context, KEY_VALUE_PRIORITY ) );
                    }
                }
            }

            if( ( tagName.equals( "event" ) || tagName.equals( "fire_event" ) )
                && keyName.equals( "name" ) ) {
                // add events
                List< String > events = new ArrayList< String >( );
                events.addAll( TemplateProvider.getInstance( )
                    .getCAC( "events" ) );
                events.addAll( projectCache_.getEvents( ) );

                for( String event: events ) {
                    acceptor
                        .accept( createCompletionProposal( event, context ) );
                }
            }
            else {
                final int nodeOffset = NodeModelUtils.getNode( model )
                    .getTotalOffset( );
                List< String > variables = new ArrayList< String >( );

                // add CAC variables
                variables.addAll( TemplateProvider.getInstance( ).getCAC(
                    "variables" ) );

                // filter variables by index
                Collection< String > projectVariables = Collections2.transform(
                    projectCache_.getVariables( ).values( ),
                    new Function< WMLVariable, String >( ) {

                        @Override
                        public String apply( WMLVariable from )
                        {
                            for( Scope scope: from.getScopes( ) ) {
                                if( scope.contains( dependencyIndex_,
                                    nodeOffset ) ) {
                                    return from.getName( );
                                }
                            }

                            return null;
                        }
                    } );

                variables.addAll( Collections2.filter( projectVariables,
                    Predicates.notNull( ) ) );

                for( String variable: variables ) {
                    acceptor.accept( createCompletionProposal( "$" + variable,
                        context ) );
                }
            }
        }
    }

    private void addKeyNameProposals( EObject model,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        WMLTag tag = null;
        if( model instanceof WMLTag ) {
            tag = ( WMLTag ) model;
        }
        else if( model.eContainer( ) instanceof WMLTag ) {
            tag = ( WMLTag ) model.eContainer( );
        }

        if( tag != null ) {
            WMLTag schemaTag = schemaParser_.getTags( ).get( tag.getName( ) );
            // try getting the custom ones
            if( schemaTag == null ) {
                schemaTag = projectCache_.getWMLTags( ).get( tag.getName( ) );
            }

            if( schemaTag != null ) {
                for( WMLKey key: schemaTag.getWMLKeys( ) ) {
                    // skip forbidden keys
                    if( key.is_Forbidden( ) ) {
                        continue;
                    }

                    boolean toAdd = true;
                    // check only non-repeatable keys
                    if( ! key.is_Repeatable( ) ) {
                        // don't suggest already completed keys
                        toAdd = ( WMLUtils.getKeyByName( tag, key.getName( ) ) != null );
                    }

                    if( toAdd ) {
                        acceptor.accept( createCompletionProposal(
                            key.getName( ) + "=", //$NON-NLS-1$
                            key.getName( ), WML_KEY_IMAGE, context,
                            KEY_NAME_PRIORITY ) );
                    }
                }
            }
        }
    }

    private void addTagProposals( EObject model, boolean ruleProposal,
        ContentAssistContext context, ICompletionProposalAcceptor acceptor )
    {
        WMLTag parentTag = null;
        if( model instanceof WMLTag ) {
            parentTag = ( WMLTag ) model;
        }
        else if( model.eContainer( ) instanceof WMLTag ) {
            parentTag = ( WMLTag ) model.eContainer( );
        }

        boolean appendEndBracket =
            context.getCurrentNode( ) == null ||
                ! context.getCurrentNode( ).getText( ).equals( "]" );

        if( parentTag != null ) {
            ICompositeNode node = NodeModelUtils.getNode( model );

            String parentIndent = ""; //$NON-NLS-1$
            if( context.getCurrentNode( ).getOffset( ) > 0 ) {
                parentIndent = NodeModelUtils.findLeafNodeAtOffset( node,
                    context.getCurrentNode( ).getTotalOffset( ) -
                        ( appendEndBracket ? 1: 2 ) ).getText( );
            }

            // remove ugly new lines that break indentation
            parentIndent = parentIndent.replace( "\r", "" ).replace( "\n", "" ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$

            WMLTag tagChildren = schemaParser_.getTags( ).get(
                parentTag.getName( ) );
            if( tagChildren != null ) {
                boolean toAdd = true;
                for( WMLTag tag: tagChildren.getWMLTags( ) ) {
                    // skip forbidden tags
                    if( tag.is_Forbidden( ) ) {
                        continue;
                    }

                    toAdd = true;

                    // check only non-repeatable tags
                    if( ! tag.is_Repeatable( ) ) {
                        toAdd = ( WMLUtils.getTagByName( parentTag,
                            tag.getName( ) ) == null );
                    }

                    if( toAdd ) {
                        acceptor.accept( createTagProposal( tag.asWMLTag( ),
                            parentIndent, ruleProposal, context,
                            appendEndBracket ) );
                    }
                }
            }
        }
        else // we are at the root
        {
            WMLTag rootTag = schemaParser_.getTags( ).get( "root" ); //$NON-NLS-1$
            for( WMLTag tag: rootTag.getWMLTags( ) ) {
                acceptor.accept( createTagProposal( tag,
                    "", ruleProposal, context, appendEndBracket ) ); //$NON-NLS-1$
            }
        }

        // parsed custom tags
        for( WMLTag tag: projectCache_.getWMLTags( ).values( ) ) {
            acceptor.accept(
                createTagProposal( tag, "", ruleProposal, context, //$NON-NLS-1$
                    appendEndBracket ) );
        }
    }

    /**
     * Returns the proposal for the specified tag, usign the specified indent
     * 
     * @param tag
     *        The tag from which to construct the proposal
     * @param indent
     *        The indent used to indent the tag and subsequent keys
     * @param ruleProposal
     *        Whether this is a proposal for an entire rule or not
     * @param context
     * @param appendEndBracked
     * @return
     */
    private ICompletionProposal createTagProposal( WMLTag tag, String indent,
        boolean ruleProposal, ContentAssistContext context,
        boolean appendEndBracked )
    {
        StringBuilder proposal = new StringBuilder( );
        if( ruleProposal ) {
            proposal.append( "[" ); //$NON-NLS-1$
        }
        proposal.append( tag.getName( ) );
        proposal.append( "]\n" ); //$NON-NLS-1$
        for( WMLKey key: tag.getWMLKeys( ) ) {
            if( key.is_Required( ) ) {
                proposal.append( String.format( "\t%s%s=\n", //$NON-NLS-1$
                    indent, key.getName( ) ) );
            }
        }
        proposal.append( String.format( "%s[/%s%s",//$NON-NLS-1$
            indent,
            tag.getName( ),
            appendEndBracked ? "]": "" ) );

        return createCompletionProposal( proposal.toString( ), tag.getName( ),
            WML_TAG_IMAGE, context, TAG_PRIORITY );
    }

    private ICompletionProposal createCompletionProposal( String proposal,
        ContentAssistContext context, int priority )
    {
        return createCompletionProposal( proposal, null, null, priority,
            context.getPrefix( ), context );
    }

    private ICompletionProposal createCompletionProposal( String proposal,
        String displayString, Image image,
        ContentAssistContext contentAssistContext, int priority )
    {
        return createCompletionProposal( proposal, new StyledString(
            displayString ), image, priority,
            contentAssistContext.getPrefix( ), contentAssistContext );
    }
}

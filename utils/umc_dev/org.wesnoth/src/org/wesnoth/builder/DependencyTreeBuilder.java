/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.LinkedBlockingDeque;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.emf.common.util.BasicEList;
import org.eclipse.emf.common.util.EList;
import org.wesnoth.Logger;
import org.wesnoth.builder.WesnothProjectBuilder.WMLFilesComparator;
import org.wesnoth.projects.ProjectDependencyNode;
import org.wesnoth.utils.ListUtils;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLRoot;
import org.wesnoth.wml.WMLTag;

public class DependencyTreeBuilder
{
    protected IProject project_;
    protected int currentIndex_ = 0;
    private ProjectDependencyNode parent_;
    private ProjectDependencyNode previous_;

    protected Map< String, ProjectDependencyNode > tree_;

    public DependencyTreeBuilder( IProject project )
    {
        tree_ = new HashMap<String, ProjectDependencyNode>();
        project_ = project;
    }

    public void createDependencyTree()
    {
        // start creating the PDT (project dependency tree)
        Queue<IContainer> containers = new LinkedBlockingDeque<IContainer>( );

        parent_ = null;

        containers.add( project_ );

        while( containers.isEmpty( ) == false ) {
            IContainer container = containers.poll( );

            IResource main_cfg = container.findMember( "_main.cfg" );
            if ( main_cfg != null ) {
                // add main.cfg to tree
                addNode( (IFile) main_cfg );

                WMLRoot root = ResourceUtils.getWMLRoot( ( IFile ) main_cfg );

                // nothing to do
                if ( root == null )
                    continue;

                EList<WMLMacroCall> macroCalls = new BasicEList<WMLMacroCall>( );

                // iterate to find macro calls
                // - search in tags
                // - search in ifdefs

                Queue<WMLTag> tags = new LinkedBlockingDeque<WMLTag>( root.getTags( ) );
                Queue<WMLPreprocIF> ifdefs = new LinkedBlockingDeque<WMLPreprocIF>( root.getIfDefs( ) );

                // add first the root defines
                macroCalls.addAll( root.getMacroCalls( ) );

                while ( !tags.isEmpty( ) ||
                        !ifdefs.isEmpty( ) ) {

                    if ( !tags.isEmpty( ) ) {
                        WMLTag tag = tags.poll( );

                        tags.addAll( tag.getTags( ) );
                        ifdefs.addAll( tag.getIfDefs( ) );

                        // now add contained macro calls
                        macroCalls.addAll( tag.getMacroCalls( ) );

                    } else {
                        WMLPreprocIF ifdef = ifdefs.poll( );

                        tags.addAll( ifdef.getTags( ) );
                        ifdefs.addAll( ifdef.getIfDefs( ) );

                        macroCalls.addAll( ifdef.getMacroCalls( ) );
                    }
                }

                // now check what macros are really an inclusion macro
                Set<String> containersToAdd = new HashSet<String>( );

                for ( WMLMacroCall macro : macroCalls ) {
                    String name = macro.getName( );

                    /**
                     * To include a folder the macro should be the following
                     * forms:
                     * - {campaigns/... }
                     * - {~add-ons/... }
                     *
                     */
                    //TODO: check for including a specific config file?

                    if ( ( name.equals( "campaigns" ) ||
                         name.equals( "add-ons" ) ) &&
                         // the call should contain just string values
                         macro.getExtraMacros( ).isEmpty( ) &&
                         macro.getParams( ).size( ) > 1 &&
                         macro.getParams( ).get( 0 ).equals( "/" ) )
                    {
                        // check if the macro includes directories local
                        // to this project
                        String projectPath = project_.getLocation( ).toOSString( );

                        if ( projectPath.contains( macro.getParams( ).get( 1 ) ) ) {
                            containersToAdd.add(
                                ListUtils.concatenateList(
                                   macro.getParams( ).subList( 2, macro.getParams( ).size( ) ), "" ) );
                        }
                    }
                }

                // push the containers in the queue
                for ( String containerPath : containersToAdd ) {
                    containers.offer( project_.getFolder( containerPath ) );
                }

            }else {
                // no main.cfg, just follow WML reading rules

                List<IResource> members = null;
                try {
                    members = Arrays.asList( container.members( ) );
                }
                catch ( CoreException e ) {
                    Logger.getInstance( ).logException( e );

                    continue;
                }

                Collections.sort( members, new WMLFilesComparator( ) );

                if ( members.isEmpty( ) )
                    continue;

                previous_ = null;

                for ( IResource resource : members ) {
                    if ( resource instanceof IContainer )
                        containers.add( (IContainer)resource );
                    else {
                        String fileName = resource.getName( );

                        // just config files.
                        if ( ! fileName.endsWith( ".cfg" ) ||
                             ! ( resource instanceof IFile ))
                            continue;

                        addNode( ( IFile ) resource );
                    }
                }
            }
        }

        System.out.println("tree:");
        if ( !tree_.isEmpty( ) ) {
            ProjectDependencyNode node = tree_.get( "_ROOT_" ); // $NON-NLS-1$

            do {
                System.out.print( "> " );
                ProjectDependencyNode leaf = node;

                do {
                    System.out.print( leaf + "; " );
                    leaf = leaf.getNext( );
                } while ( leaf != null );

                node = node.getSon( );
                System.out.print("\n");
            }while ( node != null );
        }
        else {
            System.out.println("Empty");
        }
    }

    private void addNode( IFile file )
    {
        ProjectDependencyNode newNode = new ProjectDependencyNode( file, currentIndex_ );
        currentIndex_ += ProjectDependencyNode.INDEX_STEP;

        if ( previous_ != null ){
            previous_.setNext( newNode );
            newNode.setPrevious( previous_ );
        } else {
            // first node in the current directory -> make it son of parent
            if ( parent_ != null ) {
                parent_.setSon( newNode );
                newNode.setParent( parent_ );
            } else {
                // no parent yet (== null)
                // so we're making this the root node for this tree
                tree_.put( "_ROOT_", newNode );
            }

            parent_ = newNode;
        }

        tree_.put( file.getProjectRelativePath( ).toString( ), newNode );
        previous_ = newNode;
    }
}

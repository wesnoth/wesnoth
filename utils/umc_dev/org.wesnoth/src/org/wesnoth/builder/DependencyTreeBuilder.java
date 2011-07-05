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
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingDeque;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.wesnoth.Logger;
import org.wesnoth.builder.WesnothProjectBuilder.WMLFilesComparator;
import org.wesnoth.projects.ProjectDependencyNode;

public class DependencyTreeBuilder
{
    protected IProject project_;
    protected int currentIndex_ = 0;

    protected Map< String, ProjectDependencyNode > tree_;

    public DependencyTreeBuilder( IProject project )
    {
        tree_ = new HashMap<String, ProjectDependencyNode>();
        project_ = project;
    }

    public void createDependencyTree()
    {
        // start creating the PDT (project dependency tree)
        Queue<IContainer> toProcess = new LinkedBlockingDeque<IContainer>( );

        ProjectDependencyNode parent = null;

        toProcess.add( project_ );

        while( toProcess.isEmpty( ) == false ) {
            IContainer container = toProcess.poll( );

            IResource main_cfg = container.findMember( "_main.cfg" );
            if ( main_cfg != null ) {
                // add main.cfg to tree
                //WMLRoot root = ResourceUtils.getWMLRoot( ( IFile ) main_cfg );

                // iterate to find macro calls that include other dirs
            }else {
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

                ProjectDependencyNode previous = null;

                for ( IResource resource : members ) {
                    System.out.println( resource.toString( ) );
                    if ( resource instanceof IContainer )
                        toProcess.add( (IContainer)resource );
                    else {
                        String fileName = resource.getName( );

                        // just config files.
                        if ( ! fileName.endsWith( ".cfg" ) ||
                             ! ( resource instanceof IFile ))
                            continue;

                        addNode( ( IFile ) resource, previous, parent );
                    }
                }
            }
        }

        System.out.println("tree:");
        if ( !tree_.isEmpty( ) ) {
            ProjectDependencyNode node = tree_.get( "_ROOT_" ); // $NON-NLS-1$

            do {
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

    private void addNode( IFile file, ProjectDependencyNode previous,
            ProjectDependencyNode parent )
    {

        ProjectDependencyNode newNode = new ProjectDependencyNode( file, currentIndex_ );
        currentIndex_ += ProjectDependencyNode.INDEX_STEP;

        if ( previous != null ){
            previous.setNext( newNode );
            newNode.setPrevious( previous );
        } else {
            // first node in the current directory -> make it son of parent
            if ( parent != null ) {
                parent.setSon( newNode );
                newNode.setParent( parent );
            } else {
                // no parent yet (== null)
                // so we're making this the root node for this tree
                tree_.put( "_ROOT_", newNode );
            }

            parent = newNode;
        }

        tree_.put( file.getProjectRelativePath( ).toString( ), newNode );
        previous = newNode;
    }
}

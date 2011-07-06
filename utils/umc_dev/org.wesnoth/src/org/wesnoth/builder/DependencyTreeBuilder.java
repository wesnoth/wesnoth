/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.Serializable;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashSet;
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
import org.eclipse.emf.common.util.TreeIterator;
import org.eclipse.emf.ecore.EObject;
import org.wesnoth.Logger;
import org.wesnoth.builder.WesnothProjectBuilder.WMLFilesComparator;
import org.wesnoth.utils.ListUtils;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLRoot;

public class DependencyTreeBuilder implements Serializable
{
    private static final long serialVersionUID = 6007509520015856611L;

    protected transient IProject project_;
    protected int currentIndex_ = 0;
    private ProjectDependencyNode parent_;
    private ProjectDependencyNode previous_;

    protected Map< String, ProjectDependencyNode > tree_;

    public DependencyTreeBuilder( IProject project )
    {
        tree_ = new HashMap<String, ProjectDependencyNode>();
        project_ = project;
    }

    /**
     * Create the whole dependency tree from scratch
     */
    public void createDependencyTree()
    {
        // start creating the PDT (project dependency tree)
        Queue<IContainer> containers = new LinkedBlockingDeque<IContainer>( );

        parent_ = null;
        tree_.clear( );

        containers.add( project_ );

        while( containers.isEmpty( ) == false ) {
            IContainer container = containers.poll( );

            IResource main_cfg = container.findMember( "_main.cfg" ); //$NON-NLS-1$
            if ( main_cfg != null ) {
                // add main.cfg to tree
                addNode( (IFile) main_cfg );

                WMLRoot root = ResourceUtils.getWMLRoot( ( IFile ) main_cfg );
                // nothing to do
                if ( root == null )
                    continue;

                EList<WMLMacroCall> macroCalls = new BasicEList<WMLMacroCall>( );

                // iterate to find macro calls
                TreeIterator<EObject> treeItor = root.eAllContents( );

                while ( treeItor.hasNext( ) ) {
                    EObject object = treeItor.next( );
                    if ( object instanceof WMLMacroCall ){
                        macroCalls.add( (WMLMacroCall) object );
                    }
                }

                // now check what macros are really an inclusion macro
                Set<String> containersToAdd = new LinkedHashSet<String>( );

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

                    if ( ( name.equals( "campaigns" ) || //$NON-NLS-1$
                         name.equals( "add-ons" ) ) && //$NON-NLS-1$
                         // the call should contain just string values
                         macro.getExtraMacros( ).isEmpty( ) &&
                         macro.getParams( ).size( ) > 1 &&
                         macro.getParams( ).get( 0 ).equals( "/" ) ) //$NON-NLS-1$
                    {
                        // check if the macro includes directories local
                        // to this project
                        String projectPath = project_.getLocation( ).toOSString( );

                        if ( projectPath.contains( macro.getParams( ).get( 1 ) ) ) {
                            containersToAdd.add(
                                ListUtils.concatenateList(
                                   macro.getParams( ).subList( 2, macro.getParams( ).size( ) ), "" ) ); //$NON-NLS-1$
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
                        if ( ! fileName.endsWith( ".cfg" ) || //$NON-NLS-1$
                             ! ( resource instanceof IFile ))
                            continue;

                        addNode( ( IFile ) resource );
                    }
                }
            }
        }

        System.out.println("tree:"); //$NON-NLS-1$
        if ( !tree_.isEmpty( ) ) {
            ProjectDependencyNode node = tree_.get( "_ROOT_" ); // $NON-NLS-1$ //$NON-NLS-1$

            do {
                System.out.print( "> " ); //$NON-NLS-1$
                ProjectDependencyNode leaf = node;

                do {
                    System.out.print( leaf + "; " ); //$NON-NLS-1$
                    leaf = leaf.getNext( );
                } while ( leaf != null );

                node = node.getSon( );
                System.out.print("\n"); //$NON-NLS-1$
            }while ( node != null );
        }
        else {
            System.out.println("Empty"); //$NON-NLS-1$
        }
    }

    /**
     * Adds a new node to this tree
     * @param file The file to add
     */
    public void addNode( IFile file )
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
                tree_.put( "_ROOT_", newNode ); //$NON-NLS-1$
            }

            parent_ = newNode;
        }

        tree_.put( file.getProjectRelativePath( ).toString( ), newNode );
        previous_ = newNode;
    }

    /**
     * Removes a node specified by the file
     * @param file The file to remove from the tree
     */
    public void removeNode( IFile file )
    {

    }

    /**
     * Deserializes this object from the input
     * @param input The object input stream
     * @throws IOException
     * @throws ClassNotFoundException
     */
    public void deserialize( ObjectInputStream input ) throws IOException, ClassNotFoundException
    {
        DependencyTreeBuilder tmp = (DependencyTreeBuilder) input.readObject( );
        if ( tmp == null )
            return;

        this.currentIndex_ = tmp.currentIndex_;
        this.tree_ = tmp.tree_;
        this.previous_ = tmp.previous_;
        this.parent_ = tmp.parent_;

        // now, refill the dependency nodes
        for ( ProjectDependencyNode node : tree_.values( ) ) {
            node.file_ = project_.getFile( node.fileName_ );
        }
    }
}

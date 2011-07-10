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
import java.util.ArrayList;
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
import org.wesnoth.utils.ListUtils;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.ResourceUtils.WMLFilesComparator;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLRoot;

public class DependencyListBuilder implements Serializable
{
    private static final long serialVersionUID = 6007509520015856611L;
    /**
     * The key by which the root node of the list is memorized
     * in the list.
     */
    public static final String ROOT_NODE_KEY = "_ROOT_";

    protected transient IProject project_;

    protected boolean isCreated_;
    protected int currentIndex_;
    private DependencyListNode previous_;

    protected Map< String, DependencyListNode > list_;
    protected List< String > directories_;

    public DependencyListBuilder( IProject project )
    {
        list_ = new HashMap<String, DependencyListNode>();
        directories_ = new ArrayList<String>();

        previous_ = null;

        project_ = project;
        isCreated_ = false;
        currentIndex_ = 0;
    }

    /**
     * Create the whole dependency list from scratch.
     * @param force True for force re-creating the list even if it
     * was previously created
     */
    public void createDependencyList( boolean force )
    {
        if ( isCreated_ && !force ) {
            Logger.getInstance( ).log( " Skipping depedency list for project " +
                    project_.getName( ) );
            return;
        }

        isCreated_ = true;
        currentIndex_ = 0;
        previous_ = null;
        list_.clear( );

        // start creating the PDL (project dependency List)
        Queue<IContainer> containers = new LinkedBlockingDeque<IContainer>( );

        containers.add( project_ );

        while( containers.isEmpty( ) == false ) {
            IContainer container = containers.poll( );

            IResource main_cfg = container.findMember( "_main.cfg" ); //$NON-NLS-1$
            if ( main_cfg != null ) {
                // add main.cfg to list
                internal_addNode( (IFile) main_cfg );

                Set<String> containersToAdd = getContainers( (IFile) main_cfg );
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

                Collections.sort( members, new WMLFilesComparator() );

                if ( members.isEmpty( ) )
                    continue;

                previous_ = null;

                for ( IResource resource : members ) {
                    if ( resource instanceof IContainer )
                        containers.add( (IContainer)resource );
                    else {
                        // just config files.
                        if ( !ResourceUtils.isConfigFile( resource ) )
                            continue;

                        internal_addNode( ( IFile ) resource );
                    }
                }
            }
        }

        System.out.println( toString( ) );
    }

    public static Set<String> getContainers( IFile file )
    {
        IProject project = file.getProject( );
        WMLRoot root = ResourceUtils.getWMLRoot( file );
        // nothing to do
        if ( root == null )
            return new LinkedHashSet<String> ( 0 );

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
                String projectPath = project.getLocation( ).toOSString( );

                if ( projectPath.contains( macro.getParams( ).get( 1 ) ) ) {
                    containersToAdd.add(
                        ListUtils.concatenateList(
                           macro.getParams( ).subList( 2, macro.getParams( ).size( ) ), "" ) ); //$NON-NLS-1$
                }
            }
        }

        return containersToAdd;
    }

    public DependencyListNode addNode( IFile file )
    {
        // save current nodes
        DependencyListNode previousBak = previous_, newNode = null ;

        // find the correct previous and parent to place the new node
        String parentPath = file.getParent( ).getProjectRelativePath( ).
            removeTrailingSeparator( ).toString( );
        String fileName = file.getName( );

        DependencyListNode root = getNode( ROOT_NODE_KEY );

        while ( root != null ) {

            if ( root.getFile( ).getParent( ).getProjectRelativePath( ).
                    removeTrailingSeparator( ).toString( ).
                    equals( parentPath.toString( ) ) ) {

                // found the directory. Now find the place
                DependencyListNode leaf = root;
                while ( leaf != null ) {

                    // we found the place?
                    if ( ResourceUtils.wmlFileNameCompare(
                            fileName,
                            leaf.getFile( ).getName( ) ) < 0 ) {

                        previous_ = leaf.getPrevious( );

                        newNode = internal_addNode( file );

                        // update links
                        newNode.setNext( leaf );
                        leaf.setPrevious( newNode );
                        break;
                    }

                    leaf = leaf.getNext( );
                }

                break;
            }

            root = root.getNext( );
        }

        // didn't found any place to put it. where shall we?
        //TODO: the place should be dictated by other cfg,
        // by getting the included directory
        if ( newNode == null ) {

        }

        // restore nodes
        previous_ = previousBak;

        // print the new list
        System.out.println( toString( ) );
        return newNode;
    }

    /**
     * Adds a new node to this list
     * @param file The file to add
     */
    private DependencyListNode internal_addNode( IFile file )
    {
        DependencyListNode newNode = new DependencyListNode( file, -1 );

        if ( previous_ != null ){

            // inserting is done between 2 nodes
            if ( previous_.getNext( ) != null ){
                newNode.setIndex(
                        (previous_.getIndex( ) +
                         previous_.getNext( ).getIndex( )) / 2 );

                newNode.setNext( previous_.getNext( ) );
                previous_.getNext( ).setPrevious( newNode );
            } else {
                newNode.setIndex( currentIndex_ );
                currentIndex_ += DependencyListNode.INDEX_STEP;
            }

            previous_.setNext( newNode );
            newNode.setPrevious( previous_ );
        } else {
            // no previous yet (== null)
            // so we're making this the root node for this list

            // check if we had a previous root node
            DependencyListNode root = list_.get( ROOT_NODE_KEY );
            if ( root != null ) {
                root.setPrevious( newNode );
                newNode.setNext( root );

                newNode.setIndex( root.getIndex( ) - DependencyListNode.INDEX_STEP );
            } else {
                newNode.setIndex( currentIndex_ );
                currentIndex_ += DependencyListNode.INDEX_STEP;
            }

            list_.put( ROOT_NODE_KEY, newNode ); //$NON-NLS-1$
        }

        list_.put( file.getProjectRelativePath( ).toString( ), newNode );
        previous_ = newNode;
        return newNode;
    }

    /**
     * Removes a node specified by the file
     * @param file The file to remove from the list
     */
    public void removeNode( IFile file )
    {
        DependencyListNode node = getNode( file );

        // the node didn't even exist in the list!?
        if ( node == null )
            return;

        if ( node.getPrevious( ) != null )
            node.getPrevious( ).setNext( node.getNext( ) );
        if ( node.getNext( ) != null )
            node.getNext( ).setPrevious( node.getPrevious( ) );

        list_.remove( file.getProjectRelativePath( ).toString( ) );
    }

    /**
     * Returns the node specified by the file
     * @param file The file to get the depedency node for
     * @return An instance of {@link DependencyListNode}
     */
    public DependencyListNode getNode( IFile file )
    {
        return list_.get( file.getProjectRelativePath( ).toString( ) );
    }

    /**
     * Returns the node specified by the key. The keys are
     * usually project-relative paths for project's files, or
     * the {@link #ROOT_NODE_KEY}
     * @param key The key to get the node by
     * @return An instance of {@link DependencyListNode}
     */
    public DependencyListNode getNode ( String key )
    {
        return list_.get( key );
    }

    /**
     * Returns true if the list was already created, false otherwise
     * @return A boolean value
     */
    public boolean getIsCreated()
    {
        return isCreated_;
    }

    /**
     * Deserializes this object from the input
     * @param input The object input stream
     * @throws IOException
     * @throws ClassNotFoundException
     */
    public void deserialize( ObjectInputStream input ) throws IOException, ClassNotFoundException
    {
        DependencyListBuilder tmp = (DependencyListBuilder) input.readObject( );
        if ( tmp == null )
            return;

        this.currentIndex_ = tmp.currentIndex_;
        this.isCreated_ = tmp.isCreated_;
        this.list_ = tmp.list_;
        this.previous_ = tmp.previous_;

        // now, refill the dependency nodes
        for ( DependencyListNode node : list_.values( ) ) {
            node.file_ = project_.getFile( node.fileName_ );
        }
    }

    @Override
    public String toString()
    {
        StringBuilder str = new StringBuilder( );
        str.append( "list: \n" ); //$NON-NLS-1$
        if ( !list_.isEmpty( ) ) {
            DependencyListNode node = list_.get( ROOT_NODE_KEY );

            do {
                str.append( node + "; " ); //$NON-NLS-1$

                node = node.getNext( );
                str.append( "\n" ); //$NON-NLS-1$
            }while ( node != null );
        }
        else {
            str.append( "Empty\n" ); //$NON-NLS-1$
        }

        return str.toString( );
    }
}

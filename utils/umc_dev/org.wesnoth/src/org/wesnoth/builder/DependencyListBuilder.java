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
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

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

    /**
     * Holds a list of directories that are parsed in the WML order
     * (that is, they don't have a _main.cfg in them) and
     * the value is the first node in that directory existing in the list
     */
    protected List< String > directories_;
    protected List< DependencyListNode > directoriesEntries_;

    public DependencyListBuilder( IProject project )
    {
        list_ = new HashMap<String, DependencyListNode>();
        directories_ = new ArrayList<String>();
        directoriesEntries_ = new ArrayList<DependencyListNode>();

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
            Logger.getInstance( ).log( "Skipping depedency list for project " +
                    project_.getName( ) );
            return;
        }

        isCreated_ = true;
        currentIndex_ = 0;
        previous_ = null;
        list_.clear( );
        directories_.clear( );
        directoriesEntries_.clear( );

        internal_addContainer( null );

        System.out.println( toString( ) );
    }

    /**
     * Adds a new node in the PDL
     * @param file The file to add
     * @return The newly created node
     */
    public DependencyListNode addNode( IFile file )
    {
        DependencyListNode backupPrevious = previous_, newNode = null;
        String fileProjectPath = file.getParent( ).getProjectRelativePath( ).toString( );

        // we add a file in an existing processed directory.
        if ( directories_.contains( fileProjectPath ) ) {

            int dirEntryIndex = directories_.indexOf( fileProjectPath );

            /* TODO: check if the file is _main.cfg. If yes,
             * all current nodes from this directory need to be erased
             * and processed by the includes from other _main.cfg
             */
            DependencyListNode tmpNode = directoriesEntries_.get( dirEntryIndex );

            // had any files in dir?
            if ( tmpNode != null ) {
                // search for the correct place in current dir
                String fileName = file.getName( );

                DependencyListNode prevTmpNode = null;
                while ( tmpNode != null ) {

                    // we found the place?
                    if ( ResourceUtils.wmlFileNameCompare(
                            tmpNode.getFile( ).getName( ), fileName  ) > 0 ) {

                        previous_ = tmpNode.getPrevious( );

                        newNode = internal_addNode( file );
                        break;
                    }

                    prevTmpNode = tmpNode;
                    tmpNode = tmpNode.getNext( );
                }

                // we arrived at the end
                if ( newNode == null ) {
                    previous_ = prevTmpNode;
                    newNode = internal_addNode( file );
                }
            } else {

                // previous_ should be the first non-null node in previous
                // directories.

                while ( dirEntryIndex > 0 && tmpNode == null ) {
                    -- dirEntryIndex;
                    tmpNode = directoriesEntries_.get( dirEntryIndex );
                }

                previous_ = tmpNode;
                newNode = internal_addNode( file );
            }

            // now, parse the file to check if we should include other dirs
            internal_addContainers( getContainers( file ) );
        } else {
            // didn't found any place to put it. where shall we?

            //TODO: the place should be dictated by other cfg,
            // by getting the included directory
        }

        // restore old previous
        previous_ = backupPrevious;

        // print the new list
        System.out.println( toString( ) );
        return newNode;
    }

    /**
     * Adds the containers and their contents to the list
     * @param containerList The list of container paths
     */
    private void internal_addContainers( Collection<String> containerList )
    {
        for ( String container : containerList ) {
            internal_addContainer( container );
        }
    }

    /**
     * Add the container and it's contents to the list
     * @param containerPath The path of the container
     */
    private void internal_addContainer( String containerPath )
    {
        IContainer container = null ;
        if ( containerPath == null )
            container = project_;
        else
            container = project_.getFolder( containerPath );

        IResource main_cfg = container.findMember( "_main.cfg" ); //$NON-NLS-1$
        if ( main_cfg != null ) {
            // add main.cfg to list
            internal_addNode( (IFile) main_cfg );

            // add any included containers
            internal_addContainers( getContainers( (IFile) main_cfg ) );
        }else {
            List<IResource> members = null;
            try {
                members = Arrays.asList( container.members( ) );
            }
            catch ( CoreException e ) {
                Logger.getInstance( ).logException( e );

                return;
            }

            Collections.sort( members, new WMLFilesComparator() );

            boolean toAddDirectoryEntry = false;
            // no main.cfg, just follow WML reading rules
            if ( ! directories_.contains( container.getProjectRelativePath( ).toString( ) ) ) {
                directories_.add( container.getProjectRelativePath( ).toString( ) );
                directoriesEntries_.add( null );

                toAddDirectoryEntry = true;
            }

            if ( members.isEmpty( ) )
                return;

            DependencyListNode firstNewNode = null;

            for ( IResource resource : members ) {
                if ( resource instanceof IContainer )
                    internal_addContainer( resource.getProjectRelativePath( ).toString( ) );
                else {
                    // just config files.
                    if ( !ResourceUtils.isConfigFile( resource ) )
                        continue;

                    if ( firstNewNode != null )
                        internal_addNode( ( IFile ) resource );
                    else
                        firstNewNode = internal_addNode( (IFile) resource );
                }
            }

            if ( firstNewNode != null && toAddDirectoryEntry ) {
                // update the first directory node
                directoriesEntries_.set( directories_.size( ) - 1, firstNewNode );
            }
        }
    }

    /**
     * Adds a new node to this list
     * @param file The file to add
     * @return The newly created node
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

        //debug
        System.out.println( toString( ) );
    }

    /**
     * Gets the set of included containers in this file
     * as a macro call
     * @param file The file to get the containers from
     * @return A set of containers represented by their Path as string
     */
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

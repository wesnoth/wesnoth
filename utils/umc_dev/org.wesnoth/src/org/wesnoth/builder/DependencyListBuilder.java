/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;

import org.wesnoth.Logger;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.ResourceUtils.WMLFilesComparator;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.views.WesnothProjectsExplorer;

public class DependencyListBuilder implements Serializable
{
    private static final long                 serialVersionUID = 6007509520015856611L;
    /**
     * The key by which the root node of the list is memorized
     * in the list.
     */
    public static final String                ROOT_NODE_KEY    = "_ROOT_";

    private transient IProject                project_;

    private boolean                           isCreated_;
    private int                               currentIndex_;
    private DependencyListNode                previous_;

    private Map< String, DependencyListNode > list_;

    /**
     * Holds a list of directories that are parsed in the WML order
     * (that is, they don't have a _main.cfg in them) and
     * the value is the first node in that directory existing in the list
     */
    private List< String >                    directories_;
    private List< ListDirectoryEntry >        directoriesEntries_;

    public DependencyListBuilder( IProject project )
    {
        list_ = new HashMap< String, DependencyListNode >( );
        directories_ = new ArrayList< String >( );

        directoriesEntries_ = new ArrayList< ListDirectoryEntry >( );

        previous_ = null;

        project_ = project;
        isCreated_ = false;
        currentIndex_ = 0;
    }

    /**
     * Create the whole dependency list from scratch.
     * 
     * @param force
     *        True for force re-creating the list even if it
     *        was previously created
     */
    public void createDependencyList( boolean force )
    {
        if( isCreated_ && ! force ) {
            Logger.getInstance( )
                .log( "Skipping depedency list for project "
                    + project_.getName( ) );
            return;
        }

        isCreated_ = true;
        currentIndex_ = 0;
        previous_ = null;
        list_.clear( );
        directories_.clear( );
        directoriesEntries_.clear( );

        internal_addContainer( project_.getProjectRelativePath( ).toString( ) );
    }

    /**
     * Adds a new node in the PDL
     * 
     * @param file
     *        The file to add
     * @return The newly created node
     */
    public DependencyListNode addNode( IFile file )
    {
        // TODO: create better heuristics for the indexes
        // in case we need to add multiple subsequent files...
        DependencyListNode backupPrevious = previous_, newNode = null;
        String fileParentProjectPath = file.getParent( )
            .getProjectRelativePath( ).toString( );

        // we add a file in an existing processed directory.
        if( directories_.contains( fileParentProjectPath ) ) {

            int dirEntryIndex = directories_.indexOf( fileParentProjectPath );

            ListDirectoryEntry entry = directoriesEntries_.get( dirEntryIndex );
            DependencyListNode tmpNode = entry.FirstNode;

            // had any files in dir?
            if( tmpNode != null ) {
                String fileName = file.getName( );

                /*
                 * check if the file is _main.cfg. If yes,
                 * all current nodes from this directory need to be erased
                 * and processed by the includes from other _main.cfg
                 */
                if( fileName.equals( "_main.cfg" ) ) {

                    // save the previous
                    previous_ = tmpNode.getPrevious( );

                    internal_removeContainer( fileParentProjectPath );

                    // create the node
                    newNode = internal_addNode( file );
                }
                else {
                    // search for the correct place in current dir
                    DependencyListNode prevTmpNode = null;
                    while( tmpNode != null ) {

                        // we found the place?
                        if( ResourceUtils.wmlFileNameCompare( tmpNode.getFile( )
                            .getName( ), fileName ) > 0 ) {

                            previous_ = tmpNode.getPrevious( );

                            newNode = internal_addNode( file );
                            break;
                        }

                        prevTmpNode = tmpNode;
                        tmpNode = tmpNode.getNext( );
                    }

                    // we arrived at the end
                    if( newNode == null ) {
                        previous_ = prevTmpNode;
                        newNode = internal_addNode( file );
                    }
                }
            }
            else {

                // previous_ should be the first non-null node in previous
                // directories.

                while( dirEntryIndex > 0 && tmpNode == null ) {
                    --dirEntryIndex;
                    tmpNode = directoriesEntries_.get( dirEntryIndex ).FirstNode;
                }

                previous_ = tmpNode;
                newNode = internal_addNode( file );
            }

            // now, parse the file to check if we should include other dirs
            internal_addContainers( newNode.getIncludes( true ) );
        }
        else {
            // didn't found any place to put it. where shall we?
            // the place should be dictated by other cfg,
            // by getting the included directory
        }

        // restore old previous
        previous_ = backupPrevious;

        return newNode;
    }

    /**
     * Adds the containers and their contents to the list
     * 
     * @param containerList
     *        The list of container paths
     */
    private void internal_addContainers( Collection< String > containerList )
    {
        for( String container: containerList ) {
            internal_addContainer( container );
        }
    }

    /**
     * Add the container and it's contents to the list
     * 
     * @param containerPath
     *        The path of the container
     */
    private void internal_addContainer( String containerPath )
    {
        IContainer container = null;
        if( StringUtils.isNullOrEmpty( containerPath ) ) {
            container = project_;
        }
        else {
            container = project_.getFolder( containerPath );
        }

        // skip core library
        if( container.getName( ).equals(
            WesnothProjectsExplorer.CORE_LIBRARY_NAME ) ) {
            return;
        }

        IResource main_cfg = container.findMember( "_main.cfg" ); //$NON-NLS-1$
        if( main_cfg != null ) {
            // add main.cfg to list
            DependencyListNode newNode = internal_addNode( ( IFile ) main_cfg );

            // add any included containers
            internal_addContainers( newNode.getIncludes( true ) );
        }
        else {
            // no main.cfg, just follow WML reading rules
            List< IResource > members = null;
            try {
                members = Arrays.asList( container.members( ) );
            } catch( CoreException e ) {
                Logger.getInstance( ).logException( e );
                return;
            }

            Collections.sort( members, new WMLFilesComparator( ) );

            boolean toAddDirectoryEntry = false;
            if( ! directories_.contains( containerPath ) ) {
                directories_.add( containerPath );
                directoriesEntries_.add( new ListDirectoryEntry( containerPath,
                    null, null ) );

                toAddDirectoryEntry = true;
            }
            else {
                // update the includes
                directoriesEntries_.get( directoriesEntries_
                    .indexOf( containerPath ) ).Includes++;
            }

            if( members.isEmpty( ) ) {
                return;
            }

            DependencyListNode firstNewNode = null;
            DependencyListNode lastNode = null;

            for( IResource resource: members ) {
                if( resource instanceof IContainer ) {
                    internal_addContainer( resource.getProjectRelativePath( )
                        .toString( ) );
                }
                else {
                    // just config files.
                    if( ! ResourceUtils.isConfigFile( resource ) ) {
                        continue;
                    }

                    lastNode = internal_addNode( ( IFile ) resource );
                    if( firstNewNode == null ) {
                        firstNewNode = lastNode;
                    }
                }
            }

            if( firstNewNode != null && lastNode != null ) {
                if( toAddDirectoryEntry ) {
                    // update the first directory node
                    directoriesEntries_.set( directories_.size( ) - 1,
                        new ListDirectoryEntry( containerPath,
                            firstNewNode, lastNode ) );
                }
                else {
                    // if the current entry has null nodes
                    // or the indexes are greater/lower than the current ones
                    // we need to update the references nodes

                    ListDirectoryEntry entry = directoriesEntries_
                        .get( directories_.indexOf( containerPath ) );

                    if( entry.FirstNode == null
                        || ( entry.FirstNode != null && firstNewNode
                            .getIndex( ) < entry.FirstNode.getIndex( ) ) ) {
                        entry.FirstNode = firstNewNode;
                    }

                    if( entry.LastNode == null
                        || ( entry.LastNode != null && lastNode.getIndex( ) > entry.LastNode
                            .getIndex( ) ) ) {
                        entry.LastNode = lastNode;
                    }
                }
            }
        }
    }

    /**
     * Adds a new node to this list
     * 
     * @param file
     *        The file to add
     * @return The newly created node
     */
    private DependencyListNode internal_addNode( IFile file )
    {
        DependencyListNode newNode = new DependencyListNode( file, - 1 );

        if( previous_ != null ) {

            // inserting is done between 2 nodes
            if( previous_.getNext( ) != null ) {
                int newIndex = ( previous_.getIndex( ) + previous_.getNext( )
                    .getIndex( ) ) / 2;

                if( newIndex > previous_.getIndex( )
                    + DependencyListNode.INDEX_STEP ) {
                    newIndex = previous_.getIndex( )
                        + DependencyListNode.INDEX_STEP;
                }

                newNode.setIndex( newIndex );

                newNode.setNext( previous_.getNext( ) );
                previous_.getNext( ).setPrevious( newNode );
            }
            else {
                newNode.setIndex( currentIndex_ );
                currentIndex_ += DependencyListNode.INDEX_STEP;
            }

            previous_.setNext( newNode );
            newNode.setPrevious( previous_ );
        }
        else {
            // no previous yet (== null)
            // so we're making this the root node for this list

            // check if we had a previous root node
            DependencyListNode root = list_.get( ROOT_NODE_KEY );
            if( root != null ) {
                root.setPrevious( newNode );
                newNode.setNext( root );

                newNode.setIndex( root.getIndex( )
                    - DependencyListNode.INDEX_STEP );
            }
            else {
                newNode.setIndex( currentIndex_ );
                currentIndex_ += DependencyListNode.INDEX_STEP;
            }

            list_.put( ROOT_NODE_KEY, newNode );
        }

        list_.put( file.getProjectRelativePath( ).toString( ), newNode );
        previous_ = newNode;
        return newNode;
    }

    /**
     * Removes a node specified by the file
     * 
     * @param file
     *        The file to remove from the list
     */
    public void removeNode( IFile file )
    {
        removeNode( getNode( file ) );
    }

    /**
     * Removes the specified node from the list
     * 
     * @param node
     *        The node to remove from the list
     */
    public void removeNode( DependencyListNode node )
    {
        // the node didn't even exist in the list!?
        if( node == null ) {
            return;
        }

        if( node.getPrevious( ) != null ) {
            node.getPrevious( ).setNext( node.getNext( ) );
        }
        if( node.getNext( ) != null ) {
            node.getNext( ).setPrevious( node.getPrevious( ) );
        }

        String fileParentProjectPath = node.getFile( ).getParent( )
            .getProjectRelativePath( ).toString( );

        list_.remove( node.getFile( ).getProjectRelativePath( ).toString( ) );

        // if we're at last node, decrease currentIndex_ to make economy on
        // indexes
        if( node.getNext( ) == null ) {
            if( node.getPrevious( ) != null ) {
                currentIndex_ = node.getPrevious( ).getIndex( )
                    + DependencyListNode.INDEX_STEP;
            }
        }

        // removing a _main.cfg, add the parent container
        // back to the list along with it's directories_ entry
        if( node.getFile( ).getName( ).equals( "_main.cfg" ) ) {
            DependencyListNode backupPrevious = previous_;

            previous_ = node.getPrevious( );
            internal_addContainer( fileParentProjectPath );

            previous_ = backupPrevious;
        }
    }

    /**
     * Removes the container and all it's contents from the list
     * 
     * @param path
     *        The container's path
     */
    private void internal_removeContainer( String path )
    {
        int dirEntryIndex = directories_.indexOf( path );
        if( dirEntryIndex == - 1 ) {
            return;
        }

        ListDirectoryEntry entry = directoriesEntries_.get( dirEntryIndex );

        if( entry == null ) {
            return;
        }

        --entry.Includes;

        // we shouldn't delete entries that are used more than 1 time
        if( entry.Includes > 0 ) {
            return;
        }

        DependencyListNode firstNode = entry.FirstNode;
        DependencyListNode lastNode = entry.LastNode.getNext( );

        // now delete all nodes that are in the same folder
        while( firstNode != null && firstNode != lastNode ) {
            removeNode( firstNode );
            firstNode = firstNode.getNext( );
        }

        // clear the directories entry since we can't use it anymore
        directories_.remove( dirEntryIndex );
        directoriesEntries_.remove( dirEntryIndex );
    }

    /**
     * Updates the current node in the list
     * 
     * @param node
     *        The node to update
     */
    public void updateNode( DependencyListNode node )
    {
        // check the includes, to see if they changed
        List< String > previousIncludes = node.getIncludes( false );
        int prevLength = previousIncludes.size( );

        List< String > newIncludes = node.getIncludes( true );
        int newLength = newIncludes.size( );

        List< String > processedIncludes = new ArrayList< String >( );

        for( int prevIndex = 0, newIndex = 0; prevIndex < prevLength
            && newIndex < newLength; ) {
            String prevIncl = previousIncludes.get( prevIndex );
            String newIncl = newIncludes.get( prevIndex );

            // nothing changed
            if( prevIncl.equals( newIncl ) ) {
                ++prevIndex;
                ++newIndex;
                continue;
            }

            boolean newIsNew = ! previousIncludes.contains( newIncl );
            boolean prevDeleted = ! newIncludes.contains( prevIncl );

            if( newIsNew ) {
                // add the new directory before the previous included dir (if
                // any)

                DependencyListNode backupPrevious = previous_;

                previous_ = null;

                // get the directory entry for the previous include
                if( newIndex > 0 ) {
                    ListDirectoryEntry entry = directoriesEntries_
                        .get( directories_.indexOf( newIncludes
                            .get( newIndex - 1 ) ) );

                    if( entry != null ) {
                        previous_ = entry.FirstNode;
                    }
                }

                internal_addContainer( newIncl );

                previous_ = backupPrevious;
                ++newIndex;
            }
            else {

                if( prevDeleted ) {
                    // the previous include was deleted
                    internal_removeContainer( prevIncl );
                }
                else {
                    // the previous included has changed it's index (in the
                    // includes list)

                    // don't reprocess this pair if we already did
                    if( ! processedIncludes.contains( prevIncl ) ) {

                        ListDirectoryEntry prevEntry = directoriesEntries_
                            .get( directories_.indexOf( prevIncl ) );
                        ListDirectoryEntry newEntry = directoriesEntries_
                            .get( directories_.indexOf( newIncl ) );

                        if( prevEntry != null && newEntry != null
                            && prevEntry.FirstNode != null
                            && newEntry.FirstNode != null ) {

                            // create a list for easier swap
                            List< DependencyListNode > nodes = new ArrayList< DependencyListNode >( );
                            nodes.add( prevEntry.FirstNode.getPrevious( ) );
                            nodes.add( prevEntry.FirstNode );
                            nodes.add( prevEntry.LastNode );
                            if( prevEntry.LastNode.getNext( ) != newEntry.FirstNode ) {
                                nodes.add( prevEntry.LastNode.getNext( ) );
                                nodes.add( newEntry.FirstNode.getPrevious( ) );
                            }
                            nodes.add( newEntry.FirstNode );
                            nodes.add( newEntry.LastNode );
                            nodes.add( newEntry.LastNode.getNext( ) );

                            int nodesSize = nodes.size( );

                            int swapIndex = ( nodesSize == 8 ? 5: 3 );

                            // now swap the nodes
                            DependencyListNode tmp = nodes.get( swapIndex );
                            nodes.set( swapIndex, nodes.get( 1 ) );
                            nodes.set( 1, tmp );

                            swapIndex = ( nodesSize == 8 ? 6: 4 );

                            tmp = nodes.get( swapIndex );
                            nodes.set( swapIndex, nodes.get( 2 ) );
                            nodes.set( 2, tmp );

                            // update the links

                            for( int i = 0; i < nodesSize - 1; i += 2 ) {
                                DependencyListNode fst = nodes.get( i );
                                DependencyListNode lst = nodes.get( i + 1 );

                                if( fst != null ) {
                                    fst.setNext( lst );
                                }

                                if( lst != null ) {
                                    lst.setPrevious( fst );
                                }
                            }

                            processedIncludes.add( prevIncl );
                            processedIncludes.add( newIncl );
                        }
                        else {
                            Logger.getInstance( ).log(
                                "Null directory entry for" + "includes: "
                                    + prevIncl + " and " + newIncl );
                        }
                    }
                }

                ++newIndex;
                ++prevIndex;
            }
        }
    }

    /**
     * Returns the node specified by the file
     * 
     * @param file
     *        The file to get the depedency node for
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
     * 
     * @param key
     *        The key to get the node by
     * @return An instance of {@link DependencyListNode}
     */
    public DependencyListNode getNode( String key )
    {
        return list_.get( key );
    }

    /**
     * Returns true if the list was already created, false otherwise
     * 
     * @return A boolean value
     */
    public boolean getIsCreated( )
    {
        return isCreated_;
    }

    /**
     * Deserializes this object with the specified project
     */
    public void deserialize( IProject project )
    {
        project_ = project;

        // now, refill the dependency nodes
        for( DependencyListNode node: list_.values( ) ) {
            node.file_ = project_.getFile( node.fileName_ );
        }
    }

    @Override
    public String toString( )
    {
        StringBuilder str = new StringBuilder( );
        str.append( "list: \n" ); //$NON-NLS-1$
        if( ! list_.isEmpty( ) ) {
            DependencyListNode node = list_.get( ROOT_NODE_KEY );

            do {
                str.append( node + "; " ); //$NON-NLS-1$

                node = node.getNext( );
                str.append( "\n" ); //$NON-NLS-1$
            } while( node != null );
        }
        else {
            str.append( "Empty\n" ); //$NON-NLS-1$
        }

        return str.toString( );
    }

    /**
     * The class that represents the entry in the list of included directories
     * 
     */
    protected static class ListDirectoryEntry implements Serializable
    {
        private static final long serialVersionUID = 4721697818923147755L;

        /**
         * The project relative path of the directory
         */
        public String             Name;

        /**
         * The first node in this directory existing in the list
         */
        public DependencyListNode FirstNode;

        /**
         * The last node in this directory existing in the list
         */
        public DependencyListNode LastNode;

        /**
         * Numbers of times this directory is included.
         */
        public int                Includes;

        public ListDirectoryEntry( String name, DependencyListNode firstNode,
            DependencyListNode lastNode )
        {
            Name = name;
            FirstNode = firstNode;
            LastNode = lastNode;
            Includes = 1;
        }
    }
}

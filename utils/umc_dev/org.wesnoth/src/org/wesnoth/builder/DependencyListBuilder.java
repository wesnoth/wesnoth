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

/**
 * The builder that creates a dependency list for a project
 */
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
     * Stores a list to list nodes for each included config file.
     */
    private Map< String, DependencyListNode > fileIncludes_;

    /**
     * Holds a list of directories that are parsed in the WML order
     * (that is, they don't have a _main.cfg in them).
     */
    private List< String >                    directories_;

    /**
     * This list contains the first node of each directory in the
     * {@link #directories_} list
     */
    private List< DirectoryIncludeEntry >     directoriesEntries_;

    /**
     * Creates a new {@link DependencyListBuilder} instance for the
     * specified project
     * 
     * @param project
     *        The project to create the list for
     */
    public DependencyListBuilder( IProject project )
    {
        list_ = new HashMap< String, DependencyListNode >( );
        fileIncludes_ = new HashMap< String, DependencyListNode >( );

        directories_ = new ArrayList< String >( );
        directoriesEntries_ = new ArrayList< DirectoryIncludeEntry >( );

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
        if( ! fileParentProjectPath.isEmpty( ) ) {
            fileParentProjectPath = "/" + fileParentProjectPath;
        }

        // we add a file in an existing processed directory.
        int dirEntryIndex = directories_.indexOf( fileParentProjectPath );
        if( dirEntryIndex != - 1 ) {

            DirectoryIncludeEntry entry = directoriesEntries_
                .get( dirEntryIndex );
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

            // now, parse the file to check if we should include other
            // dirs/files
            internal_addIncludes( newNode.getMacroIncludes( true ) );
        }
        else {
            // didn't found any place to put it. where shall we?
            // the place should be dictated by other cfg via a macro include
        }

        // restore old previous
        previous_ = backupPrevious;

        return newNode;
    }

    /**
     * Adds the includes to the list
     * 
     * @param containerList
     *        The list of container paths
     */
    private void internal_addIncludes( Collection< String > includesList )
    {
        for( String include: includesList ) {
            internal_addInclude( include );
        }
    }

    /**
     * Adds the include to the list. The include can be either a file
     * or a folder, but this is resolved automatically.
     * 
     * @param include
     *        The include to add
     */
    private void internal_addInclude( String include )
    {
        IFile file = project_.getFile( include );
        if( file.exists( ) ) {
            DependencyListNode node = internal_addNode( file );
            // save the include
            fileIncludes_.put( include, node );
        }
        else {
            internal_addContainer( include );
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

            // add any included files/folders
            internal_addIncludes( newNode.getMacroIncludes( true ) );
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
                directoriesEntries_.add( new DirectoryIncludeEntry(
                    containerPath,
                    null, null ) );

                toAddDirectoryEntry = true;
            }
            else {
                // update the includes
                for( DirectoryIncludeEntry entry: directoriesEntries_ ) {
                    if( entry.DirectoryPath.equals( containerPath ) ) {
                        ++entry.Includes;
                        break;
                    }
                }
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
                        new DirectoryIncludeEntry( containerPath,
                            firstNewNode, lastNode ) );
                }
                else {
                    // if the current entry has null nodes
                    // or the indexes are greater/lower than the current ones
                    // we need to update the references nodes

                    DirectoryIncludeEntry entry = directoriesEntries_
                        .get( directories_.indexOf( containerPath ) );

                    if( entry.FirstNode == null
                        || firstNewNode.getIndex( ) < entry.FirstNode
                            .getIndex( ) ) {
                        entry.FirstNode = firstNewNode;
                    }

                    if( entry.LastNode == null
                        || lastNode.getIndex( ) > entry.LastNode.getIndex( ) ) {
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
        // don't add a file more than 1 time
        if( list_.containsKey( file.getProjectRelativePath( ).toString( ) ) ) {
            return getNode( file );
        }

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

        IFile file = node.getFile( );

        if( node.equals( list_.get( ROOT_NODE_KEY ) ) ) {
            list_.remove( ROOT_NODE_KEY );
        }

        list_.remove( file.getProjectRelativePath( ).toString( ) );

        // if we're at last node, decrease currentIndex_ to make economy on
        // indexes
        if( node.getNext( ) == null ) {
            if( node.getPrevious( ) != null ) {
                currentIndex_ = node.getPrevious( ).getIndex( )
                    + DependencyListNode.INDEX_STEP;
            }
        }

        // if removing a _main.cfg, we need to add the parent container
        // back to the list along with it's directories_ entry
        if( file.getName( ).equals( "_main.cfg" ) ) {
            DependencyListNode backupPrevious = previous_;

            previous_ = node.getPrevious( );
            internal_addContainer( file.getParent( ).getProjectRelativePath( )
                .toString( ) );

            previous_ = backupPrevious;
        }
    }

    /**
     * Removes the specified include
     * 
     * @param include
     *        The include to remove
     */
    private void internal_removeInclude( String include )
    {
        IFile file = project_.getFile( include );
        if( file.exists( ) ) {
            removeNode( file );
        }
        else {
            internal_removeContainer( include );
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

        DirectoryIncludeEntry entry = directoriesEntries_.get( dirEntryIndex );

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
     * Updates the specified node in the list
     * 
     * @param node
     *        The node to update
     */
    public void updateNode( DependencyListNode node )
    {
        // check the includes, to see if they changed
        List< String > previousIncludes = node.getMacroIncludes( false );
        int prevLength = previousIncludes.size( );

        List< String > newIncludes = node.getMacroIncludes( true );
        int newLength = newIncludes.size( );

        List< String > processedIncludes = new ArrayList< String >( );

        for( int prevIndex = 0, newIndex = 0; prevIndex < prevLength
            || newIndex < newLength; ) {
            String prevInclude = null;
            String newInclude = null;
            if( prevIndex < prevLength ) {
                prevInclude = previousIncludes.get( prevIndex );
            }
            if( newIndex < newLength ) {
                newInclude = newIncludes.get( newIndex );
            }

            // nothing changed
            if( prevInclude != null &&
                prevInclude.equals( newInclude ) ) {
                ++prevIndex;
                ++newIndex;
                continue;
            }

            boolean newIsNew = prevIndex >= prevLength ||
                ( newInclude != null &&
                ! previousIncludes.contains( newInclude ) );
            boolean prevDeleted = newIndex >= newLength ||
                ( prevInclude != null &&
                ! newIncludes.contains( prevInclude ) );

            if( newIsNew ) {
                // add the new include before the previous included dir (if
                // any)

                DependencyListNode backupPrevious = previous_;

                previous_ = null;

                // find the node for the previous include
                if( newIndex > 0 ) {

                    int dirIndex = directories_.indexOf( newIncludes
                        .get( newIndex - 1 ) );

                    if( dirIndex != - 1 ) {
                        // previous include was a directory
                        DirectoryIncludeEntry entry = directoriesEntries_
                            .get( dirIndex );

                        if( entry != null ) {
                            previous_ = entry.LastNode;
                        }
                    }
                    else {
                        previous_ = fileIncludes_.get( newIncludes
                            .get( newIndex - 1 ) );
                    }
                }

                internal_addInclude( newInclude );

                previous_ = backupPrevious;
                ++newIndex;
            }
            else if( prevDeleted ) {
                // the previous include was deleted
                // it's not present in the new includes list
                internal_removeInclude( prevInclude );

                ++prevIndex;
            }
            else {
                // the previous include has changed it's index (in the
                // includes list)
                // there are 3 types of swaps:
                // 1 ) file - file
                // 2 ) directory - file
                // 3 ) directory - directory

                // don't reprocess this pair if we already did
                if( ! processedIncludes.contains( prevInclude ) ) {

                    // see what type of swap we need to do.

                    int prevDirIndex = directories_.indexOf( prevInclude );
                    int newDirIndex = directories_.indexOf( newInclude );

                    if( prevDirIndex == - 1 && newDirIndex == - 1 ) {
                        // file <-> file. Just swap the nodes
                        DependencyListNode prevNode = fileIncludes_
                            .get( prevInclude );
                        DependencyListNode newNode = fileIncludes_
                            .get( newInclude );

                        DependencyListNode tmpSwapNode = prevNode.getPrevious( );
                        prevNode.setPrevious( newNode.getPrevious( ) );
                        if( newNode.getPrevious( ) != null ) {
                            newNode.getPrevious( ).setNext( prevNode );
                        }
                        newNode.setPrevious( tmpSwapNode );
                        if( tmpSwapNode != null ) {
                            tmpSwapNode.setNext( newNode );
                        }

                        tmpSwapNode = prevNode.getNext( );
                        prevNode.setNext( newNode.getNext( ) );
                        if( newNode.getNext( ) != null ) {
                            newNode.getNext( ).setPrevious( prevNode );
                        }
                        newNode.setNext( tmpSwapNode );
                        if( tmpSwapNode != null ) {
                            tmpSwapNode.setPrevious( newNode );
                        }
                    }
                    else {
                        // directory <-> directory and
                        // directory <-> file
                        // This is code is a bit ugly and weird, but for the
                        // moment I haven't found any better solution
                        // If you want to understand it, you'd better use
                        // a pen and paper. That's what I did when writing it!
                        DirectoryIncludeEntry prevEntry = null;
                        DirectoryIncludeEntry newEntry = null;
                        DependencyListNode prevNode = null;
                        DependencyListNode newNode = null;
                        DependencyListNode middleNode = null;

                        boolean prevIsDir = false, newIsDir = false;

                        // create a list for easier swap
                        List< DependencyListNode > nodes = new ArrayList< DependencyListNode >( );

                        if( prevDirIndex != - 1 ) {
                            prevIsDir = true;
                            prevEntry = directoriesEntries_.get( prevDirIndex );

                            nodes.add( prevEntry.FirstNode.getPrevious( ) );
                            nodes.add( prevEntry.FirstNode );
                            nodes.add( prevEntry.LastNode );

                            middleNode = prevEntry.LastNode.getNext( );
                        }
                        else {
                            prevNode = fileIncludes_.get( prevInclude );

                            nodes.add( prevNode.getPrevious( ) );
                            nodes.add( prevNode );

                            middleNode = prevNode.getNext( );
                        }

                        if( newDirIndex != - 1 ) {
                            newIsDir = true;
                            newEntry = directoriesEntries_.get( newDirIndex );

                            if( middleNode != newEntry.FirstNode ) {
                                nodes.add( middleNode );
                                nodes.add( newEntry.FirstNode.getPrevious( ) );
                            }

                            nodes.add( newEntry.FirstNode );
                            nodes.add( newEntry.LastNode );
                            nodes.add( newEntry.LastNode.getNext( ) );
                        }
                        else {
                            newNode = fileIncludes_.get( newInclude );

                            if( middleNode != newNode ) {
                                nodes.add( middleNode );
                                nodes.add( newNode.getPrevious( ) );
                            }

                            nodes.add( newNode );
                            nodes.add( newNode.getNext( ) );
                        }

                        int nodesSize = nodes.size( );

                        // now swap the nodes

                        if( prevIsDir && newIsDir ) {
                            // dir <-> dir
                            int swapIndex = ( nodesSize == 8 ? 5: 3 );

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
                        }
                        else {
                            // transform the list in the oposite case
                            if( prevIsDir ) {
                                Collections.reverse( nodes );
                            }

                            List< DependencyListNode > tmpNodes =
                                new ArrayList< DependencyListNode >( nodes );

                            if( nodesSize == 7 ) {
                                nodes.set( 1, tmpNodes.get( 4 ) );
                                nodes.set( 2, tmpNodes.get( 5 ) );
                                nodes.set( 3, tmpNodes.get( 2 ) );
                                nodes.set( 4, tmpNodes.get( 3 ) );
                                nodes.set( 5, tmpNodes.get( 1 ) );
                            }
                            else {
                                nodes.set( 1, tmpNodes.get( 2 ) );
                                nodes.set( 2, tmpNodes.get( 3 ) );
                                nodes.set( 3, tmpNodes.get( 1 ) );
                            }

                            // reverse to the original order
                            if( prevIsDir ) {
                                Collections.reverse( nodes );
                            }

                            // update the links
                            for( int i = 0; i < nodesSize - 1; ) {
                                DependencyListNode fst = nodes.get( i );
                                DependencyListNode lst = nodes.get( i + 1 );

                                if( fst != null ) {
                                    fst.setNext( lst );
                                }

                                if( lst != null ) {
                                    lst.setPrevious( fst );
                                }

                                if( i == 0 && prevIsDir ) {
                                    ++i;
                                }
                                else if( i == nodesSize - 3 && ! prevIsDir ) {
                                    ++i;
                                }
                                else {
                                    i += 2;
                                }
                            }
                        }
                    }
                }

                processedIncludes.add( prevInclude );
                processedIncludes.add( newInclude );
                ++prevIndex;
                ++newIndex;
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
     * the {@link #ROOT_NODE_KEY} for the first list node
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
    public boolean isCreated( )
    {
        return isCreated_;
    }

    /**
     * Deserializes this object with the specified project
     * 
     * @param project
     *        The project to whom this list belongs to
     */
    public void deserialize( IProject project )
    {
        project_ = project;

        // now, refill the dependency nodes
        for( DependencyListNode node: list_.values( ) ) {
            node.file_ = project_.getFile( node.fileName_ );
        }
    }

    /**
     * @return The number of the nodes in the list
     */
    public int getNodesCount( )
    {
        if( list_.isEmpty( ) ) {
            return 0;
        }

        // traverse all nodes
        int size = 0;
        DependencyListNode node = list_.get( ROOT_NODE_KEY );
        while( node != null ) {
            node = node.getNext( );

            ++size;
        }

        return size;
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
    protected static class DirectoryIncludeEntry implements Serializable
    {
        private static final long serialVersionUID = 4721697818923147755L;

        /**
         * The project relative path of the directory
         */
        public String             DirectoryPath;

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

        public DirectoryIncludeEntry( String directoryPath,
            DependencyListNode firstNode,
            DependencyListNode lastNode )
        {
            DirectoryPath = directoryPath;
            FirstNode = firstNode;
            LastNode = lastNode;
            Includes = 1;
        }
    }
}

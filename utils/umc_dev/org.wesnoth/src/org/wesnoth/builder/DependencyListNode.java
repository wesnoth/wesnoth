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
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.QualifiedName;

import org.wesnoth.Logger;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.utils.ResourceUtils;

/**
 * This class represents a node in the Project's Depedency list,
 * which is constructed on a full build of the project.
 * 
 * Alternatively, a list node is created
 * when a new resource is added.
 */
public class DependencyListNode implements Serializable
{
    private static final long         serialVersionUID = - 9140173740211465384L;

    /**
     * This integer represents the default step between 2 file indexes.
     * Since int it's on 4 bytes, it can hold values between
     * -2,147,483,648 and 2,147,483,647.
     * 
     * With an increment of 100k, we could have 2 * 21,474 config files.
     */
    public static final int           INDEX_STEP       = 100000;

    /**
     * The {@link QualifiedName} that represents the Project Dependency List
     * Index
     */
    public static final QualifiedName PDL_INDEX        = new QualifiedName(
                                                           WesnothPlugin.ID_PLUGIN,
                                                           "pdl_index" );       //$NON-NLS-1$

    private DependencyListNode        previous_;
    private DependencyListNode        next_;

    protected transient IFile         file_;
    protected String                  fileName_;
    protected List< String >          includes_;

    private int                       index_;

    /**
     * Creates a new List Node
     * 
     * @param file
     *        The file contained in the node
     * @param index
     *        The index of the node
     */
    public DependencyListNode( IFile file, int index )
    {
        previous_ = next_ = null;

        includes_ = new ArrayList< String >( );

        file_ = file;
        fileName_ = file.getProjectRelativePath( ).toString( );
        setIndex( index );
    }

    /**
     * Gets this node's file
     * 
     * @return A IFile resource
     */
    public IFile getFile( )
    {
        return file_;
    }

    /**
     * Gets the includes from this node
     * 
     * @param refresh
     *        True to force reloading the current file and return
     *        the newly parsed ones
     * @return A set with string paths for included directories
     */
    public List< String > getMacroIncludes( boolean refresh )
    {
        if( includes_ == null || refresh ) {
            includes_ = new ArrayList< String >(
                ResourceUtils.getMacroIncludes( file_ ) );
        }

        return includes_;
    }

    /**
     * Returns the index of this node in the whole dependency list.
     * 
     * @return An integer index.
     */
    public int getIndex( )
    {
        return index_;
    }

    /**
     * Sets a new index for this node
     * 
     * @param index
     *        The index to set
     */
    protected void setIndex( int index )
    {
        index_ = index;

        try {
            file_.setPersistentProperty( PDL_INDEX, Integer.toString( index ) );
        } catch( CoreException e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Gets the node before the current node
     * 
     * @return A node or null if there is no parent
     */
    public DependencyListNode getPrevious( )
    {
        return previous_;
    }

    /**
     * Sets a new previous node for this node
     * 
     * @param previous
     *        The new previous node to set
     */
    public void setPrevious( DependencyListNode previous )
    {
        previous_ = previous;
    }

    /**
     * Gets the node after the current node
     * 
     * @return A node or null if there is no parent
     */
    public DependencyListNode getNext( )
    {
        return next_;
    }

    /**
     * Sets a new next node for this node
     * 
     * @param next
     *        The new next node to set
     */
    public void setNext( DependencyListNode next )
    {
        next_ = next;
    }

    @Override
    public String toString( )
    {
        return ( file_ == null ? "": fileName_ ) + "_" + index_; //$NON-NLS-1$ //$NON-NLS-2$
    }
}

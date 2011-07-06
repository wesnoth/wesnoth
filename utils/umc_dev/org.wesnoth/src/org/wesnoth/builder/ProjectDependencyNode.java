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

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.QualifiedName;
import org.wesnoth.Constants;
import org.wesnoth.Logger;

/**
 * This class represents a node
 * in the Project's Depedency tree,
 * which is constructed on a full build of the project.
 *
 * Alternatively, a tree node is created
 * when a new resource is added.
 */
public class ProjectDependencyNode implements Serializable
{
    private static final long serialVersionUID = -9140173740211465384L;

    /**
     * This integer represents the default step between 2 file indexes.
     * Since int it's on 4 bytes, it can hold values between
     * -2,147,483,648 and 2,147,483,647.
     *
     * With an increment of 10k, we could have 2*214,748 config files.
     */
    public static final int INDEX_STEP = 10000;

    protected static final QualifiedName PDT_INDEX = new QualifiedName( Constants.PLUGIN_ID, "pdt_index" ); //$NON-NLS-1$

    private ProjectDependencyNode previous_;
    private ProjectDependencyNode next_;

    private ProjectDependencyNode parent_;
    private ProjectDependencyNode son_;

    protected transient IFile file_;
    protected String fileName_;

    private int index_;

    public ProjectDependencyNode( IFile file, int index )
    {
        previous_ = next_ = parent_ = son_ = null;

        file_ = file;
        fileName_ = file.getProjectRelativePath( ).toString( );
        setIndex( index );
    }

    /**
     * Gets this node's file
     * @return A IFile resource
     */
    public IFile getFile()
    {
        return file_;
    }

    /**
     * Returns the index of this node in the whole dependency tree node
     * @return
     */
    public int getIndex()
    {
        return index_;
    }

    /**
     * Sets a new index for this node
     * @param index The index to set
     */
    public void setIndex( int index )
    {
        index_ = index;

        try {
            file_.setPersistentProperty( PDT_INDEX, Integer.toString( index ) );
        }
        catch ( CoreException e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Gets the parent of this node
     * @return A node or null if there is no parent
     */
    public ProjectDependencyNode getParent()
    {
        return parent_;
    }

    /**
     * Sets a new parent for this node
     * @param parent The parent to set
     */
    public void setParent( ProjectDependencyNode parent )
    {
        parent_ = parent;
    }

    /**
     * Gets the son of this node
     * @return A node or null if there is no parent
     */
    public ProjectDependencyNode getSon()
    {
        return son_;
    }

    /**
     * Sets a new son node for this node
     * @param son The new son node to set
     */
    public void setSon( ProjectDependencyNode son )
    {
        son_ = son;
    }

    /**
     * Gets the node before the current node
     * @return A node or null if there is no parent
     */
    public ProjectDependencyNode getPrevious()
    {
        return previous_;
    }

    /**
     * Sets a new previous node for this node
     * @param previous The new previous node to set
     */
    public void setPrevious( ProjectDependencyNode previous )
    {
        previous_ = previous;
    }

    /**
     * Gets the node after the current node
     * @return A node or null if there is no parent
     */
    public ProjectDependencyNode getNext()
    {
        return next_;
    }

    /**
     * Sets a new next node for this node
     * @param next The new next node to set
     */
    public void setNext( ProjectDependencyNode next )
    {
        next_ = next;
    }

    @Override
    public String toString()
    {
        return ( file_ == null ? "" : fileName_ ) + "_" + index_; //$NON-NLS-1$ //$NON-NLS-2$
    }
}

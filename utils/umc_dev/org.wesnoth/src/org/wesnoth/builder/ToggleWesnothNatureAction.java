/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import java.util.Iterator;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.action.IAction;

import org.wesnoth.Logger;
import org.wesnoth.action.ObjectActionDelegate;

/**
 * Toggles the Wesnoth nature on the selected project
 */
public class ToggleWesnothNatureAction extends ObjectActionDelegate
{
    @Override
    @SuppressWarnings( "rawtypes" )
    public void run( IAction action )
    {
        if( structuredSelection_ == null ) {
            return;
        }
        for( Iterator it = structuredSelection_.iterator( ); it.hasNext( ); ) {
            Object element = it.next( );
            IProject project = null;
            if( element instanceof IProject ) {
                project = ( IProject ) element;
            }
            else if( element instanceof IAdaptable ) {
                project = ( IProject ) ( ( IAdaptable ) element )
                    .getAdapter( IProject.class );
            }
            if( project != null ) {
                toggleNature( project );
            }
        }
    }

    /**
     * Toggles sample nature on a project
     * 
     * @param project
     *        to have sample nature added or removed
     */
    public void toggleNature( IProject project )
    {
        try {
            IProjectDescription description = project.getDescription( );
            String[] natures = description.getNatureIds( );

            for( int i = 0; i < natures.length; ++i ) {
                if( WesnothProjectNature.ID_NATURE.equals( natures[i] ) ) {
                    // Remove the nature
                    String[] newNatures = new String[natures.length - 1];
                    System.arraycopy( natures, 0, newNatures, 0, i );
                    System.arraycopy( natures, i + 1, newNatures, i,
                        natures.length - i - 1 );
                    description.setNatureIds( newNatures );
                    project.setDescription( description,
                        new NullProgressMonitor( ) );
                    project.refreshLocal( IResource.DEPTH_INFINITE,
                        new NullProgressMonitor( ) );
                    return;
                }
            }

            // Add the natures
            String[] newNatures = new String[natures.length + 1];
            System.arraycopy( natures, 0, newNatures, 0, natures.length );
            newNatures[natures.length] = WesnothProjectNature.ID_NATURE;
            description.setNatureIds( newNatures );
            project.setDescription( description, new NullProgressMonitor( ) );
            project.refreshLocal( IResource.DEPTH_INFINITE,
                new NullProgressMonitor( ) );
        } catch( CoreException e ) {
            Logger.getInstance( ).logException( e );
        }
    }

}

/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests.pde;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;

import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;

public class ProjectDependencyListTests extends PDETest
{

    private IProject createProject( String name ) throws CoreException
    {
        IProject project = ResourcesPlugin.getWorkspace( ).getRoot( )
            .getProject( name );

        if( project.exists( ) ) {
            project.delete( true, true, new NullProgressMonitor( ) );
        }

        project = ProjectUtils.createWesnothProject( "testproject", null,
            "default", new NullProgressMonitor( ) );
        return project;
    }

    public void testEmptyDPL( ) throws CoreException
    {
        IProject project = createProject( "test" );
        // build the project
        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        ProjectCache cache = ProjectUtils.getCacheForProject( project );

        assertEquals( true, cache.getDependencyList( ).isCreated( ) );
        assertEquals( 0, cache.getDependencyList( ).getNodesCount( ) );

        project.delete( true, true, new NullProgressMonitor( ) );
    }
}

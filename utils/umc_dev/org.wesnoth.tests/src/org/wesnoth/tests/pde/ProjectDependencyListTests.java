/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests.pde;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;

import org.wesnoth.builder.DependencyListBuilder;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.ResourceUtils;

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
            "default", true, new NullProgressMonitor( ) );
        return project;
    }

    private void cleanup( IProject project ) throws CoreException
    {
        project.delete( true, true, new NullProgressMonitor( ) );

        ProjectUtils.getProjectCaches( ).clear( );
    }

    public void testEmptyPDL( ) throws CoreException
    {
        IProject project = createProject( "test" );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );
        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );

        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 0, list.getNodesCount( ) );

        cleanup( project );
    }

    public void testSingleFile( ) throws Throwable
    {
        IProject project = createProject( "test" );

        IFile maincfg = ResourceUtils.createFile( project, "_main.cfg", "",
            true );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );
        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );

        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 1, list.getNodesCount( ) );

        assertEquals( maincfg,
            list.getNode( DependencyListBuilder.ROOT_NODE_KEY ).getFile( ) );

        cleanup( project );
    }
}

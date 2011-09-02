/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests.pde;

import java.io.FileWriter;
import java.io.IOException;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;

import org.wesnoth.builder.DependencyListBuilder;
import org.wesnoth.builder.DependencyListNode;
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

        project = ProjectUtils.createWesnothProject( name, null,
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

        IFile maincfg = ResourceUtils.createFile( project, "_main.cfg", "" );
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

    public void testFolderIncludes_OrderChanged( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[6];

        ResourceUtils.createFolder( project, "f1" );
        ResourceUtils.createFolder( project, "f2" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2/f2_filea.cfg", "" );
        files[4] = ResourceUtils.createFile( project, "f2/f2_fileb.cfg", "" );
        files[5] = ResourceUtils.createFile( project, "f2/f2_filec.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 6, list.getNodesCount( ) );

        // now reverse the include order
        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f2}\r\n{~add-ons/test/f1}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the re-ordering
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[4], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[5], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        cleanup( project );
    }

    public void testFilesIncludes( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[3];

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1.cfg}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 3, list.getNodesCount( ) );

        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        cleanup( project );
    }

    public void testFileIncludes_OrderChanged( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[3];

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1.cfg}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 3, list.getNodesCount( ) );

        // now reverse the include order
        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f2.cfg}\r\n{~add-ons/test/f1.cfg}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the re-ordering
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        cleanup( project );
    }

    /**
     * New include is added
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFileIncludes_IncludeAdded( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[3];

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 2, list.getNodesCount( ) );

        // now add a new file include
        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f1.cfg}\r\n{~add-ons/test/f2.cfg}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the new list
        assertEquals( 3, list.getNodesCount( ) );
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        cleanup( project );
    }

    /**
     * A file include is removed
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFileIncludes_IncludeRemoved( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[3];

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1.cfg}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 3, list.getNodesCount( ) );

        // now remove a file include
        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f1.cfg}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the new list
        assertEquals( 2, list.getNodesCount( ) );
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        cleanup( project );
    }

    // TODO test combined folder + file directory add/removal

    public void testIncludesCollapse( )
    {
        // add _main.cfg to an included folder
    }

    public void testIncludesExpand( )
    {
        // remove a _main.cfg from an included folder
    }

    /**
     * Test the removal of _main.cfg from the root of the project's directory
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testMainCFGRemoved( ) throws CoreException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[3];

        files[0] = ResourceUtils.createFile( project, "_main.cfg", "" );
        files[1] = ResourceUtils.createFile( project, "f1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 1, list.getNodesCount( ) );

        // now remove the _main.cfg
        files[0].delete( true, new NullProgressMonitor( ) );

        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the new list
        assertEquals( 2, list.getNodesCount( ) );

        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        cleanup( project );
    }

    /**
     * Test the addition of _main.cfg to the project's root directory
     * 
     * @throws CoreException
     */
    public void testMainCFGAdded( ) throws CoreException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[3];

        files[0] = ResourceUtils.createFile( project, "f1.cfg", "" );
        files[1] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 2, list.getNodesCount( ) );

        // now add the _main.cfg
        files[2] = ResourceUtils.createFile( project, "_main.cfg", "" );

        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the new list
        assertEquals( 1, list.getNodesCount( ) );

        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[2], node.getFile( ) );

        cleanup( project );
    }
}

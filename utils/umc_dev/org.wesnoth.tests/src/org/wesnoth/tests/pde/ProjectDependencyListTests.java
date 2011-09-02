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

    /**
     * Tests the list for an empty project
     * 
     * @throws CoreException
     */
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

    /**
     * Tests the list for a project with a single file
     * 
     * @throws Throwable
     */
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

    /**
     * Tests the modification of the folder includes order
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderIncludes_OrderChanged( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[9];

        ResourceUtils.createFolder( project, "f1" );
        ResourceUtils.createFolder( project, "f2" );
        ResourceUtils.createFolder( project, "f3" );

        files[0] = ResourceUtils
            .createFile( project, "_main.cfg",
                "{~add-ons/test/f1}\r\n{~add-ons/test/f2}\r\n{~add-ons/test/f3}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f1/f1_file3.cfg", "" );
        files[4] = ResourceUtils.createFile( project, "f2/f2_file4.cfg", "" );
        files[5] = ResourceUtils.createFile( project, "f2/f2_file5.cfg", "" );
        files[6] = ResourceUtils.createFile( project, "f3/f3_filea.cfg", "" );
        files[7] = ResourceUtils.createFile( project, "f3/f3_fileb.cfg", "" );
        files[8] = ResourceUtils.createFile( project, "f3/f3_filec.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 9, list.getNodesCount( ) );

        // now reverse the include order
        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f3}\r\n{~add-ons/test/f2}\r\n" +
            "{~add-ons/test/f1}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the reordering
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[6], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[7], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[8], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[4], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[5], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        cleanup( project );
    }

    /**
     * Tests the file includes
     * 
     * @throws CoreException
     * @throws IOException
     */
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

    /**
     * Tests the modification of the file includes order
     * 
     * @throws CoreException
     * @throws IOException
     */
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

        // check the reordering
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
     * New file include is added
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

    /**
     * Tests includes that contain both folder and file includes
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderAndFileIncludes( ) throws CoreException, IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[4];

        ResourceUtils.createFolder( project, "f1" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 4, list.getNodesCount( ) );

        // check the includes
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        cleanup( project );
    }

    /**
     * Tests the modification of the file and folder includes order
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderAndFileIncludes_OrderChanged( ) throws CoreException,
        IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[4];

        ResourceUtils.createFolder( project, "f1" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 4, list.getNodesCount( ) );

        // now reverse the include order
        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f2.cfg}\r\n{~add-ons/test/f1}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        // check the reordering
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        cleanup( project );
    }

    /**
     * New file included is added
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderAndFileIncludes_FileIncludeAdded( )
        throws CoreException, IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[5];

        ResourceUtils.createFolder( project, "f1" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2.cfg", "" );
        files[4] = ResourceUtils.createFile( project, "f3.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 4, list.getNodesCount( ) );

        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" +
            "{~add-ons/test/f3.cfg}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        assertEquals( 5, list.getNodesCount( ) );

        // check the list
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[4], node.getFile( ) );

        cleanup( project );
    }

    /**
     * File include removed
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderAndFileIncludes_FileIncludeRemoved( )
        throws CoreException, IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[5];

        ResourceUtils.createFolder( project, "f1" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" +
                "{~add-ons/test/f3.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2.cfg", "" );
        files[4] = ResourceUtils.createFile( project, "f3.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 5, list.getNodesCount( ) );

        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        assertEquals( 4, list.getNodesCount( ) );

        // check the list
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        cleanup( project );
    }

    /**
     * New folder include added
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderAndFileIncludes_FolderIncludeAdded( )
        throws CoreException, IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[6];

        ResourceUtils.createFolder( project, "f1" );
        ResourceUtils.createFolder( project, "f3" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2.cfg", "" );
        files[4] = ResourceUtils.createFile( project, "f3/f3_file1.cfg", "" );
        files[5] = ResourceUtils.createFile( project, "f3/f3_file2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 4, list.getNodesCount( ) );

        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" +
            "{~add-ons/test/f3}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        assertEquals( 6, list.getNodesCount( ) );

        // check the list
        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[4], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[5], node.getFile( ) );

        cleanup( project );
    }

    /**
     * Folder include removed
     * 
     * @throws CoreException
     * @throws IOException
     */
    public void testFolderAndFileIncludes_FolderIncludeRemoved( )
        throws CoreException, IOException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[6];

        ResourceUtils.createFolder( project, "f1" );
        ResourceUtils.createFolder( project, "f3" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" +
                "{~add-ons/test/f3}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f2.cfg", "" );
        files[4] = ResourceUtils.createFile( project, "f3/f3_file1.cfg", "" );
        files[5] = ResourceUtils.createFile( project, "f3/f3_file2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 6, list.getNodesCount( ) );

        FileWriter writer = new FileWriter( files[0].getLocation( )
            .toOSString( ) );

        writer.write( "{~add-ons/test/f1}\r\n{~add-ons/test/f2.cfg}\r\n" );
        writer.close( );
        project.refreshLocal( IResource.DEPTH_INFINITE,
            new NullProgressMonitor( ) );

        // re-build now
        project.build( IncrementalProjectBuilder.INCREMENTAL_BUILD,
            new NullProgressMonitor( ) );

        assertEquals( 4, list.getNodesCount( ) );

        DependencyListNode node = list
            .getNode( DependencyListBuilder.ROOT_NODE_KEY );
        assertEquals( files[0], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[1], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        cleanup( project );
    }

    /**
     * Tests the collapsing of the files in an included folder when
     * _main.cfg is added to it. The files nodes in that folder get
     * replaced with _main.cfg's node.
     * 
     * @throws CoreException
     */
    public void testIncludesCollapse( ) throws CoreException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[4];

        ResourceUtils.createFolder( project, "f1" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 3, list.getNodesCount( ) );

        // create _main.cfg
        files[1] = ResourceUtils.createFile( project, "f1/_main.cfg", "" );

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

    /**
     * Removing the _main.cfg from an included folder should expand that
     * _main.cfg's into the existing files from that folder.
     * 
     * @throws CoreException
     */
    public void testIncludesExpand( ) throws CoreException
    {
        IProject project = createProject( "test" );

        IFile files[] = new IFile[4];

        ResourceUtils.createFolder( project, "f1" );

        files[0] = ResourceUtils.createFile( project, "_main.cfg",
            "{~add-ons/test/f1}\r\n" );
        files[1] = ResourceUtils.createFile( project, "f1/_main.cfg", "" );
        files[2] = ResourceUtils.createFile( project, "f1/f1_file1.cfg", "" );
        files[3] = ResourceUtils.createFile( project, "f1/f1_file2.cfg", "" );

        project.build( IncrementalProjectBuilder.FULL_BUILD,
            new NullProgressMonitor( ) );
        DependencyListBuilder list = ProjectUtils.getCacheForProject( project )
            .getDependencyList( );

        assertEquals( true, list.isCreated( ) );
        assertEquals( 2, list.getNodesCount( ) );

        // now remove the _main.cfg
        files[1].delete( true, new NullProgressMonitor( ) );

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
        assertEquals( files[2], node.getFile( ) );

        node = node.getNext( );
        assertEquals( files[3], node.getFile( ) );

        cleanup( project );
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

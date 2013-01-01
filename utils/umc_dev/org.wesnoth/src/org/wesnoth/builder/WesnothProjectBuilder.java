/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingDeque;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IResourceDelta;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;

import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.preprocessor.PreprocessorUtils;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.AntUtils;
import org.wesnoth.utils.ExternalToolInvoker;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WMLTools;
import org.wesnoth.utils.WorkspaceUtils;
import org.wesnoth.views.WesnothProjectsExplorer;
import org.wesnoth.wml.SimpleWMLParser;
import org.wesnoth.wml.WMLConfig;

/**
 * The builder does the following steps in order to create and ensure
 * a correct PDL (Project Dependency Lits)
 * 
 * 1) remove REMOVED files from the PDL
 * 2) parse ADDED or CHANGED files, to check if new directory/file includes
 * happened
 * 3) add new dependencies to the list
 */
public class WesnothProjectBuilder extends IncrementalProjectBuilder
{
    /**
     * The ID of the Wesnoth Builder
     */
    public static final String ID_BUIILDER = "org.wesnoth.builders.wesnoth"; //$NON-NLS-1$

    private ProjectCache       projectCache_;
    private IProject           project_;

    /**
     * Creates a new {@link WesnothProjectBuilder}
     */
    public WesnothProjectBuilder( )
    {
        projectCache_ = null;
        project_ = null;
    }

    @SuppressWarnings( "rawtypes" )
    @Override
    protected IProject[] build( int kind, Map args, IProgressMonitor monitor )
        throws CoreException
    {
        project_ = getProject( );
        if( WesnothInstallsUtils.setupInstallForResource( project_ ) == false ) {
            return null;
        }

        if( projectCache_ == null ) {
            projectCache_ = ProjectUtils.getCacheForProject( project_ );
        }

        Logger.getInstance( ).log(
            "Building project " + getProject( ).getName( ) + " ..." ); //$NON-NLS-1$
        monitor.beginTask(
            String.format( Messages.WesnothProjectBuilder_1,
                project_.getName( ) ), 100 );

        String installName = WesnothInstallsUtils
            .getInstallNameForResource( project_ );
        Paths paths = Preferences.getPaths( installName );

        monitor.subTask( Messages.WesnothProjectBuilder_3 );
        if( WorkspaceUtils.checkPathsAreSet( installName, true ) == false ) {
            return null;
        }
        monitor.worked( 5 );

        // create the temporary directory used by the plugin if not created
        monitor.subTask( Messages.WesnothProjectBuilder_6 );
        WorkspaceUtils.getTemporaryFolder( );
        monitor.worked( 2 );


        if( runAntJob( paths, monitor ) == false ) {
            return null;
        }

        boolean readDefines = false;

        monitor.subTask( "Build started..." );

        if( kind == FULL_BUILD ) {
            readDefines = fullBuild( monitor );
        }
        else {
            IResourceDelta delta = getDelta( project_ );
            if( delta == null ) {
                readDefines = fullBuild( monitor );
            }
            else {
                readDefines = incrementalBuild( delta, monitor );
            }
        }

        if( readDefines ) {
            // we read the defines at the end of the build
            // to speed up things (and only if we had any .cfg files processed)
            projectCache_.readDefines( true );
            projectCache_.saveCache( );

            monitor.worked( 10 );
        }
        monitor.done( );

        Logger.getInstance( ).log(
            "Done building project " + getProject( ).getName( ) );
        return null;
    }

    /**
     * Does a full build on this project
     * 
     * @param monitor
     *        The monitor used to signal progress
     * @return True if there were config files processed
     * @throws CoreException
     */
    protected boolean fullBuild( final IProgressMonitor monitor )
        throws CoreException
    {
        // clean all project cache
        projectCache_.clear( );
        PreprocessorUtils.getInstance( ).clearTimestampsForPath(
            project_.getLocation( ).toOSString( ) );

        // force creating the dependency list
        projectCache_.getDependencyList( ).createDependencyList( true );
        boolean foundCfg = false;

        DependencyListNode node = null;

        node = projectCache_.getDependencyList( ).getNode(
            DependencyListBuilder.ROOT_NODE_KEY );

        if( node != null ) {

            do {
                foundCfg = true;

                // process the node
                checkResource( node.getFile( ), monitor );
                node = node.getNext( );
            } while( node != null );
        }

        // now (re-) preprocess the _main.cfg
        checkResource( project_.getFile( "_main.cfg" ), monitor );

        return foundCfg;
    }

    /**
     * Does an incremental build on this project
     * 
     * @param delta
     *        The delta which contains the project modifications
     * @param monitor
     *        The monitor used to signal progress
     * @return True if there were config files processed
     * @throws CoreException
     */
    protected boolean incrementalBuild( IResourceDelta delta,
        IProgressMonitor monitor ) throws CoreException
    {
        // Create the dependency list only if it's not created.
        projectCache_.getDependencyList( ).createDependencyList( false );
        monitor.worked( 10 );

        boolean foundCfg = false;

        // TODO: unprocessed files should be reprocessed on each build
        // until they get so.

        DependencyListBuilder list = projectCache_.getDependencyList( );
        Queue< IResourceDelta > deltasQueue = new LinkedBlockingDeque< IResourceDelta >( );
        List< DependencyListNode > nodesToProcess = new ArrayList< DependencyListNode >( );

        // gather the list of configs modified
        deltasQueue.add( delta );

        while( ! deltasQueue.isEmpty( ) ) {
            IResourceDelta deltaItem = deltasQueue.poll( );
            IResource resource = deltaItem.getResource( );

            // process just config files
            if( ResourceUtils.isConfigFile( resource ) ) {
                IFile file = ( IFile ) resource;
                int deltaKind = deltaItem.getKind( );

                if( deltaKind == IResourceDelta.REMOVED ) {
                    projectCache_.getDependencyList( ).removeNode( file );
                    projectCache_.getWMLConfigs( ).remove(
                        file.getProjectRelativePath( ).toString( ) );

                }
                else if( deltaKind == IResourceDelta.ADDED ) {
                    DependencyListNode newNode = list.addNode( file );
                    if( newNode == null ) {
                        Logger.getInstance( ).logError(
                            "Couldn't create a new" + "PDL node for file: "
                                + file.getFullPath( ).toString( ) );
                    }
                    else {
                        nodesToProcess.add( newNode );
                    }
                }
                else if( deltaKind == IResourceDelta.CHANGED ) {
                    DependencyListNode node = list.getNode( file );
                    if( node == null ) {
                        Logger.getInstance( ).logError(
                            "Couldn't find file "
                                + file.getFullPath( ).toString( )
                                + " in PDL!." );
                    }
                    else {
                        nodesToProcess.add( node );
                        list.updateNode( node );
                    }
                }
                else {
                    Logger.getInstance( ).log(
                        "unknown delta kind: " + deltaKind );
                }
            }

            // skip core library files
            if( resource instanceof IContainer
                && WesnothProjectsExplorer.CORE_LIBRARY_NAME
                    .equals( resource.getName( ) ) ) {
                continue;
            }

            deltasQueue
                .addAll( Arrays.asList( deltaItem.getAffectedChildren( ) ) );
        }

        // sort the list by index (ascending)
        Collections.sort( nodesToProcess,
            new Comparator< DependencyListNode >( ) {

                @Override
                public int compare( DependencyListNode o1,
                    DependencyListNode o2 )
                {
                    if( o1.getIndex( ) < o2.getIndex( ) ) {
                        return - 1;
                    }
                    else if( o1.getIndex( ) == o2.getIndex( ) ) {
                        return 0;
                    }

                    return 1;
                }
            } );

        foundCfg = ( ! nodesToProcess.isEmpty( ) );

        // process nodes
        for( DependencyListNode node: nodesToProcess ) {
            checkResource( node.getFile( ), monitor );
        }

        // now (re-) preprocess the _main.cfg
        checkResource( project_.getFile( "_main.cfg" ), monitor );

        return foundCfg;
    }

    /**
     * Runs the ant job that copies the project in user add-ons directory
     * 
     * @param paths
     *        The paths instance which contains the paths to wesnoth
     *        utilities
     * @param monitor
     *        The monitor used to signal progress
     * @return true or false whether the job completed successfully
     */
    private boolean runAntJob( Paths paths, IProgressMonitor monitor )
    {
        String buildXMLPath = project_.getLocation( ).toOSString( )
            + "/build.xml";
        // check for 'build.xml' existance
        if( new File( buildXMLPath ).exists( ) == true ) {
            // run the ant job to copy the whole project
            // in the user add-ons directory (incremental)
            monitor.subTask( Messages.WesnothProjectBuilder_8 );
            Map< String, String > properties = new HashMap< String, String >( );
            properties.put( "wesnoth.user.dir", paths.getUserDir( ) ); //$NON-NLS-1$
            Logger.getInstance( ).logTool( "Ant result:" ); //$NON-NLS-1$

            String result = AntUtils.runAnt( buildXMLPath, properties, true );
            Logger.getInstance( ).logTool( result );
            monitor.worked( 10 );

            if( result == null ) {
                Logger.getInstance( ).log( "error running the ant job", //$NON-NLS-1$
                    Messages.WesnothProjectBuilder_13 );
                return false;
            }
        }

        monitor.worked( 2 );
        return true;
    }

    protected boolean checkResource( IResource resource,
        IProgressMonitor monitor )
    {
        monitor.worked( 5 );
        if( resource.exists( ) == false || monitor.isCanceled( ) ) {
            return false;
        }

        // config files
        if( ResourceUtils.isConfigFile( resource ) ) {
            IFile file = ( IFile ) resource;
            String filePath = file.getProjectRelativePath( ).toString( );
            String macrosFilePath = PreprocessorUtils.getInstance( )
                .getMacrosLocation( file );

            Logger.getInstance( ).log( "Resource: " + filePath ); //$NON-NLS-1$

            try {

                // preprocess the file only if there's no _main.cfg to gather
                // everything, so we preprocess individually each file
                if( ! project_.getFile( "_main.cfg" ).exists( ) ||
                    project_.getFile( "_main.cfg" ).equals( resource ) ) {
                    monitor.subTask( String.format(
                        Messages.WesnothProjectBuilder_19, filePath ) );

                    // we use a single _MACROS_.cfg file for each project
                    PreprocessorUtils.getInstance( ).preprocessFile( file,
                        macrosFilePath, new ArrayList< String >( ) );
                    monitor.worked( 5 );
                }

                // process the AST ( Abstract Syntax Tree ) to get info for the
                // file
                monitor.subTask( String.format(
                    Messages.WesnothProjectBuilder_22, filePath ) );

                WMLConfig config = projectCache_.getWMLConfig( filePath );
                SimpleWMLParser parser = new SimpleWMLParser( file, config,
                    projectCache_ );
                parser.parse( );

                monitor.worked( 10 );

            } catch( Exception e ) {
                Logger.getInstance( ).logException( e );
            }
        }
        return true;
    }

    @SuppressWarnings( "unused" )
    private void runWMLLint( String installName, IProgressMonitor monitor,
        IFile file )
    {
        monitor.subTask( String.format( "Running WMLlint on file %s ...",
            file.getName( ) ) );
        ExternalToolInvoker tool = WMLTools.runWMLLint( installName, file
            .getLocation( ).toOSString( ), false, false );
        tool.waitForTool( );

        try {
            file.deleteMarkers( Constants.MARKER_WMLLINT, false,
                IResource.DEPTH_INFINITE );
        } catch( CoreException e ) {
            Logger.getInstance( ).logException( e );
        }

        String[] output = StringUtils.getLines( tool.getErrorContent( ) );
        for( String line: output ) {
            WMLTools.parseAndAddMarkers( line, Constants.MARKER_WMLLINT );
        }

        monitor.worked( 20 );
    }

    /**
     * Run the wmlscope for the specified file
     * 
     * @param monitor
     * @param file
     * @throws CoreException
     */
    @SuppressWarnings( "unused" )
    private void runWMLScope( String installName, IProgressMonitor monitor,
        IFile file )
    {
        monitor.subTask( String.format( "Running WMLScope on file %s ...",
            file.getName( ) ) );
        ExternalToolInvoker tool = WMLTools.runWMLScope( installName, file
            .getLocation( ).toOSString( ), false );
        tool.waitForTool( );

        try {
            file.deleteMarkers( Constants.MARKER_WMLSCOPE, false,
                IResource.DEPTH_INFINITE );
        } catch( CoreException e ) {
            Logger.getInstance( ).logException( e );
        }

        String[] output = StringUtils.getLines( tool.getErrorContent( ) );
        for( String line: output ) {
            WMLTools.parseAndAddMarkers( line, Constants.MARKER_WMLSCOPE );
        }
        monitor.worked( 20 );
    }
}

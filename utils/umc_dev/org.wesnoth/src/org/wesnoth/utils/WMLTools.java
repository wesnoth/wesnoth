/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicInteger;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.console.MessageConsole;

import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;

/**
 * Utilities class that handles with WML Tools
 */
public class WMLTools
{
    /**
     * Runs "wmlindent" on the specified resource (directory/file)
     * 
     * @param installName
     *        The name of the install to use when using the wmlindent script.
     * @param resourcePath
     *        the full path of the target where "wmlindent" will be runned
     *        on
     * @param stdin
     *        the standard input string to feed "wmlindent"
     * @param dryrun
     *        true to run "wmlindent" in dry mode - i.e. no changes in the
     *        config file.
     * @param stdout
     *        The array of streams where to output the stdout content
     * @param stderr
     *        The array of streams where to output the stderr content
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker runWMLIndent( String installName,
        String resourcePath, String stdin, boolean dryrun,
        OutputStream[] stdout, OutputStream[] stderr )
    {
        Paths paths = Preferences.getPaths( installName );

        // wmlindent only check first
        if( ! checkWMLTool( paths, Tools.WMLINDENT.toString( ) ) ) {
            return null;
        }

        File wmllintFile = new File( paths.getWMLToolsDir( ) + "/wmlindent" ); //$NON-NLS-1$
        List< String > arguments = new ArrayList< String >( );
        arguments.add( wmllintFile.getAbsolutePath( ) );

        if( resourcePath != null ) {
            if( ! ResourceUtils.isValidFilePath( resourcePath ) ) {
                return null;
            }

            if( dryrun
                || Preferences.getBool( Preferences.WMLINDENT_DRYRUN ) == true ) {
                arguments.add( "--dryrun" ); //$NON-NLS-1$
            }

            if( Preferences.getBool( Preferences.WMLINDENT_VERBOSE ) ) {
                arguments.add( "-v" ); //$NON-NLS-1$
                arguments.add( "-v" ); //$NON-NLS-1$
            }
            arguments.add( resourcePath );
        }
        return runPythonScript( arguments, stdin, true, true, stdout, stderr );
    }

    /**
     * Runs "wmllint" on the specified resource (directory/file)
     * 
     * @param installName
     * 
     * @param resourcePath
     *        the full path of the target where "wmllint" will be runned on
     * @param dryrun
     *        true to run "wmllint" in dry mode - i.e. no changes in the
     *        config file.
     * @param showProgress
     *        true to show the progress of the tool
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker runWMLLint( String installName,
        String resourcePath, boolean dryrun, boolean showProgress )
    {
        return runWMLLint( installName, resourcePath, dryrun, showProgress,
            new OutputStream[0], new OutputStream[0] );
    }

    /**
     * Runs "wmllint" on the specified resource (directory/file)
     * 
     * @param installName
     *        The name of the install to use when using the wmllint script
     * @param resourcePath
     *        the full path of the target where "wmllint" will be runned on
     * @param dryrun
     *        true to run "wmllint" in dry mode - i.e. no changes in the
     *        config file.
     * @param showProgress
     *        true to show the progress of the tool
     * @param stdout
     *        The array of streams where to output the stdout content
     * @param stderr
     *        The array of streams where to output the stderr content
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker runWMLLint( String installName,
        String resourcePath, boolean dryrun, boolean showProgress,
        OutputStream[] stdout, OutputStream[] stderr )
    {
        Paths paths = Preferences.getPaths( installName );

        if( ! ResourceUtils.isValidFilePath( resourcePath )
            || ! checkWMLTool( paths, Tools.WMLLINT.toString( ) ) ) {
            return null;
        }

        File wmllintFile = new File( paths.getWMLToolsDir( ) + "/wmllint" ); //$NON-NLS-1$

        List< String > arguments = new ArrayList< String >( );

        arguments.add( wmllintFile.getAbsolutePath( ) );

        int verboseLevel = Preferences
            .getInt( Preferences.WMLLINT_VERBOSE_LEVEL );
        for( int i = 1; i <= verboseLevel; i++ ) {
            arguments.add( "-v" ); //$NON-NLS-1$
        }

        if( verboseLevel <= 0 && showProgress ) {
            arguments.add( "--progress" ); //$NON-NLS-1$
        }

        if( dryrun || Preferences.getBool( Preferences.WMLLINT_DRYRUN ) == true ) {
            arguments.add( "--dryrun" ); //$NON-NLS-1$
        }

        if( Preferences.getBool( Preferences.WMLLINT_SPELL_CHECK ) == false ) {
            arguments.add( "--nospellcheck" ); //$NON-NLS-1$
        }

        // add default core directory
        arguments.add( paths.getCoreDir( ) );
        arguments.add( resourcePath );

        return runPythonScript( arguments, null, true, true, stdout, stderr );
    }

    /**
     * Runs "wmlscope" on the specified resource (directory/file)
     * 
     * @param installName
     *        The name of the install to use when using the wmlscope script
     * @param resourcePath
     *        the full path of the target where "wmlindent" will be runned
     *        on
     * @param showProgress
     *        True to show the wmlscope progressing, or false otherwise.
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker runWMLScope( String installName,
        String resourcePath, boolean showProgress )
    {
        return runWMLScope( installName, resourcePath, showProgress,
            new OutputStream[0], new OutputStream[0] );
    }

    /**
     * Runs "wmlscope" on the specified resource (directory/file)
     * 
     * @param installName
     *        The name of the install to use when using the wmlscope script
     * @param resourcePath
     *        the full path of the target where "wmlindent" will be runned
     *        on
     * @param showProgress
     *        True to show the wmlscope progressing, or false otherwise.
     * @param stdout
     *        The array of streams where to output the stdout content
     * @param stderr
     *        The array of streams where to output the stderr content
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker runWMLScope( String installName,
        String resourcePath, boolean showProgress, OutputStream[] stdout,
        OutputStream[] stderr )
    {
        Paths paths = Preferences.getPaths( installName );

        if( ! ResourceUtils.isValidFilePath( resourcePath )
            || ! checkWMLTool( paths, Tools.WMLSCOPE.toString( ) ) ) {
            return null;
        }

        File wmlscopeFile = new File( paths.getWMLToolsDir( ) + "/wmlscope" ); //$NON-NLS-1$

        List< String > arguments = new ArrayList< String >( );

        arguments.add( wmlscopeFile.getAbsolutePath( ) );
        int verboseLevel = Preferences
            .getInt( Preferences.WMLSCOPE_VERBOSE_LEVEL );

        if( verboseLevel > 0 ) {
            arguments.add( "-w" ); //$NON-NLS-1$
            arguments.add( String.valueOf( verboseLevel ) );
        }
        else if( showProgress ) {
            arguments.add( "--progress" ); //$NON-NLS-1$
        }

        if( Preferences.getBool( Preferences.WMLSCOPE_COLLISIONS ) == true ) {
            arguments.add( "--collisions" ); //$NON-NLS-1$
        }

        arguments.add( "--unchecked" ); //$NON-NLS-1$
        arguments.add( "--unresolved" ); //$NON-NLS-1$

        // add default core directory
        arguments.add( paths.getCoreDir( ) );
        arguments.add( resourcePath );

        return runPythonScript( arguments, null, true, true, stdout, stderr );
    }

    /**
     * Runs the specified WMLTools as a workspace job
     * 
     * @param tool
     *        The tool to run.
     *        Currently only the following tools are supported: WMLLINT,
     *        WMLSCOPE, WMLINDENT
     * @param targetPath
     *        If this is not null if will use the targetpath as the
     *        argument for launching the tool
     */
    public static void runWMLToolAsWorkspaceJob( final Tools tool,
        final String targetPath )
    {
        // TODO: remove/rework this hackish method.
        if( tool == Tools.WESNOTH_ADDON_MANAGER ) {
            return;
        }

        IEditorReference[] editors = WorkspaceUtils.getWorkbenchWindow( )
            .getPages( )[0].getEditorReferences( );

        for( IEditorReference editor: editors ) {
            if( editor.isDirty( ) ) {
                editor.getEditor( false ).doSave( null );
            }
        }

        if( targetPath == null && WorkspaceUtils.getSelectedFile( ) != null ) {
            EditorUtils.openEditor( WorkspaceUtils.getSelectedFile( ), false );
        }
        final String toolName = tool.toString( );

        WorkspaceJob job = new WorkspaceJob( Messages.WMLTools_28 + toolName ) {
            private ExternalToolInvoker toolInvoker_  = null;
            private AtomicInteger       workReporter_ = new AtomicInteger( );

            @Override
            protected void canceling( )
            {
                toolInvoker_.kill( true );
                super.canceling( );
            }

            @Override
            public IStatus runInWorkspace( final IProgressMonitor monitor )
            {
                monitor.beginTask( toolName, 1050 );
                MessageConsole console = GUIUtils.createConsole( toolName
                    + Messages.WMLTools_29, null, true );
                OutputStream[] stdout = new OutputStream[] { console
                    .newMessageStream( ) };
                OutputStream[] stderr = new OutputStream[] { console
                    .newMessageStream( ) };

                String location;
                IResource resource = null;
                String installName = Preferences.getDefaultInstallName( );

                IFile selFile = WorkspaceUtils.getSelectedFile( );
                if( targetPath != null ) {
                    location = targetPath;
                }
                else {
                    if( selFile != null ) {
                        location = selFile.getLocation( ).toOSString( );
                        resource = selFile;
                    }
                    else {
                        resource = WorkspaceUtils.getSelectedContainer( );
                        location = resource.getLocation( ).toOSString( );
                    }

                    installName = WesnothInstallsUtils
                        .getInstallNameForResource( resource );
                }

                switch( tool ) {
                    case WMLINDENT:
                        if( selFile != null && targetPath == null ) {
                            String stdin = EditorUtils.getEditorDocument( )
                                .get( );
                            // don't output to stdout as we will put that in
                            // the editor
                            toolInvoker_ = WMLTools.runWMLIndent(
                                installName, null, stdin, false, null,
                                stdout );
                        }
                        else {
                            toolInvoker_ = WMLTools.runWMLIndent(
                                installName, location, null, false,
                                stdout, stderr );
                        }
                        break;
                    case WMLLINT:
                        toolInvoker_ = WMLTools.runWMLLint( installName,
                            location, true, false, stdout, stderr );
                        break;
                    case WMLSCOPE:
                        toolInvoker_ = WMLTools.runWMLScope( installName,
                            location, false, stdout, stderr );
                        break;
                }
                monitor.worked( 50 );
                // need to fill up to '1000' worked
                // we will add 1 for each 2 lines of output (for each file)
                Thread stdoutWatcher = new Thread( new Runnable( ) {
                    @Override
                    public void run( )
                    {
                        int nr;
                        while( toolInvoker_.readOutputLine( ) != null ) {
                            nr = workReporter_.incrementAndGet( );
                            if( nr % 2 == 0 ) {
                                synchronized( monitor ) {
                                    monitor.worked( 1 );
                                }
                            }
                        }
                    }
                } );
                Thread stderrWatcher = new Thread( new Runnable( ) {
                    @Override
                    public void run( )
                    {
                        int nr;
                        while( toolInvoker_.readErrorLine( ) != null ) {
                            nr = workReporter_.incrementAndGet( );
                            if( nr % 2 == 0 ) {
                                synchronized( monitor ) {
                                    monitor.worked( 1 );
                                }
                            }
                        }
                    }
                } );
                stderrWatcher.start( );
                stdoutWatcher.start( );
                toolInvoker_.waitForTool( );
                if( tool == Tools.WMLINDENT && selFile != null
                    && targetPath == null ) {
                    EditorUtils.replaceEditorText( toolInvoker_
                        .getOutputContent( ) );
                }

                if( tool == Tools.WMLSCOPE ) {
                    if( resource != null ) {
                        ResourceUtils.deleteMarkers( resource,
                            Constants.MARKER_WMLSCOPE );
                    }
                    parseAndAddMarkers( toolInvoker_.getOutputContent( ),
                        Constants.MARKER_WMLSCOPE );
                }
                else if( tool == Tools.WMLLINT ) {
                    if( resource != null ) {
                        ResourceUtils.deleteMarkers( resource,
                            Constants.MARKER_WMLLINT );
                    }
                    parseAndAddMarkers( toolInvoker_.getOutputContent( ),
                        Constants.MARKER_WMLLINT );
                }

                monitor.worked( 50 );
                monitor.done( );
                return Status.OK_STATUS;
            }
        };
        job.schedule( );
    }

    /**
     * Runs the wesnoth addon manager for uploading the specified container
     * 
     * @param containerPath
     *        The container to upload
     * @param stdout
     *        The array of streams where to output the stdout content
     * @param stderr
     *        The array of streams where to output the stderr content
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker uploadWesnothAddon( String containerPath,
        OutputStream[] stdout, OutputStream[] stderr )
    {
        if( ! ResourceUtils.isValidFilePath( containerPath ) ) {
            return null;
        }

        return runWesnothAddonManager(
            WesnothInstallsUtils.getInstallNameForResource( containerPath ),
            Preferences.getString( Preferences.ADDON_MANAGER_PASSWORD ),
            Preferences.getString( Preferences.ADDON_MANAGER_PORT ),
            Arrays.asList( "-u", containerPath ) // upload container
            , stdout, stderr );
    }

    /**
     * Runs the wesnoth addon manager with the specified parameters
     * 
     * @param installName
     *        The install of which to use the wesnoth addon manager from
     * @param password
     *        The password to access the addons server
     * @param port
     *        The port of the addons server
     * @param extraArguments
     *        The extra arguments to the addons manager
     * @param stdout
     *        The array of streams where to output the stdout content
     * @param stderr
     *        The array of streams where to output the stderr content
     * @return null if there were errors or an ExternalToolInvoker instance
     */
    public static ExternalToolInvoker runWesnothAddonManager(
        String installName, String password, String port,
        List< String > extraArguments, OutputStream[] stdout,
        OutputStream[] stderr )
    {
        Paths paths = Preferences.getPaths( installName );
        if( ! checkWMLTool( paths, Tools.WESNOTH_ADDON_MANAGER.toString( ) ) ) {
            return null;
        }

        File wmllintFile = new File( paths.getWMLToolsDir( )
            + "/wesnoth_addon_manager" ); //$NON-NLS-1$
        List< String > arguments = new ArrayList< String >( );
        arguments.add( wmllintFile.getAbsolutePath( ) );

        if( ! StringUtils.isNullOrEmpty( password ) ) {
            arguments.add( "-P" ); //$NON-NLS-1$
            arguments.add( password );
        }

        if( Preferences.getBool( Preferences.ADDON_MANAGER_VERBOSE ) == true ) {
            arguments.add( "-V" ); //$NON-NLS-1$
        }

        arguments.add( "-a" ); //$NON-NLS-1$
        arguments.add( Preferences
            .getString( Preferences.ADDON_MANAGER_ADDRESS ) );

        arguments.add( "-p" ); //$NON-NLS-1$
        arguments.add( port );

        arguments.addAll( extraArguments );

        return runPythonScript( arguments, null, true, true, stdout, stderr );
    }

    /**
     * Parses the output from the WMLTool and adds markers accordingly
     * 
     * @param output
     *        The output to parse and add the markers from.
     * @param markerType
     *        The type of markers to add from the parsed tokens.
     */
    public static void parseAndAddMarkers( String output, String markerType )
    {
        String[] lines = StringUtils.getLines( output );
        boolean startMD5 = false;

        for( String line: lines ) {
            if( line.startsWith( "#" ) || //$NON-NLS-1$
                line.matches( "^[\\t ]*$" ) || //$NON-NLS-1$
                line.startsWith( "wmllint:" ) ) {
                continue;
            }
            if( line.startsWith( "%%" ) ) //$NON-NLS-1$
            {
                startMD5 = ! startMD5;
                continue;
            }
            // skip parsing collisions
            if( startMD5 ) {
                continue;
            }

            ResourceUtils.addMarkerForLine( line, markerType );
        }
    }

    /**
     * Checks if the specified wmlTool is existing an can
     * be runned (python path is valid aswell)
     * 
     * @param paths
     *        The paths for current install
     * @param wmlTool
     *        The wml tool to check
     * @return True if tool is ok to be used
     */
    public static boolean checkWMLTool( Paths paths, String wmlTool )
    {
        String pythonPath = Preferences.getString( Preferences.PYTHON_PATH );
        if( pythonPath.isEmpty( )
            || ( pythonPath.matches( "^.*(/|\\\\).*$" ) && ! ResourceUtils
                .isValidFilePath( Preferences
                    .getString( Preferences.PYTHON_PATH ) ) ) ) {
            GUIUtils.showWarnMessageBox( Messages.WMLTools_42 );
            return false;
        }

        if( wmlTool != null ) {
            if( paths.getWMLToolsDir( ).equals( "" ) ) //$NON-NLS-1$
            {
                GUIUtils.showWarnMessageBox( Messages.WMLTools_45 );
                return false;
            }

            File wmlToolFile = new File( paths.getWMLToolsDir( ) + wmlTool );

            if( ! wmlToolFile.exists( ) ) {
                GUIUtils.showErrorMessageBox( String.format(
                    Messages.WMLTools_47, wmlToolFile ) );
                return false;
            }

            return true;
        }

        return false;
    }

    /**
     * Runs a specified python script with the specified arguments
     * (the call returns immediately)
     * 
     * @param arguments
     *        the arguments of the "python" executable.
     *        The first argument should be the script file name
     * @param stdin
     *        A string that will be written to stdin of the python script
     * @param stdout
     *        An array of streams where to write the stdout from the script.
     *        A non null array implies 'stdoutMonitoring' true.
     * @param stderr
     *        An array of streams where to write the stderr from the script
     *        A non null array implies 'stderrMonitoring' true.
     * @param stderrMonitoring
     *        True to start stderr monitoring on tool
     * @param stdoutMonitoring
     *        True to start stdout monitoring on tool
     * @return An {@link ExternalToolInvoker} instance that launched the
     *         specified python script.
     */
    public static ExternalToolInvoker runPythonScript(
        List< String > arguments, String stdin, boolean stderrMonitoring,
        boolean stdoutMonitoring, final OutputStream[] stdout,
        final OutputStream[] stderr )
    {
        final ExternalToolInvoker pyscript = new ExternalToolInvoker(
            Preferences.getString( Preferences.PYTHON_PATH ), arguments );

        pyscript.runTool( );
        if( stderrMonitoring == true || ( stderr != null && stderr.length > 0 ) ) {
            pyscript.startErrorMonitor( stderr );
        }
        if( stdoutMonitoring == true || ( stdout != null && stdout.length > 0 ) ) {
            pyscript.startOutputMonitor( stdout );
        }
        if( stdin != null ) {
            try {
                BufferedWriter stdinStream = new BufferedWriter(
                    new OutputStreamWriter( pyscript.getStdin( ) ) );
                stdinStream.write( stdin );
                stdinStream.close( );
            } catch( IOException e ) {
                Logger.getInstance( ).logException( e );
            }
        }
        return pyscript;
    }

    /**
     * The available wml tools.
     */
    public enum Tools
    {
        /**
         * Wesnoth WML Lint script
         */
        WMLLINT,
            /**
             * Wesnoth WML Indent script
             */
            WMLINDENT,
            /**
             * Wesnoth WML Scope script
             */
            WMLSCOPE,
            /**
             * Wesnoth addon manager
             */
            WESNOTH_ADDON_MANAGER;

        @Override
        public String toString( )
        {
            return this.name( ).toLowerCase( Locale.ENGLISH );
        }
    }
}

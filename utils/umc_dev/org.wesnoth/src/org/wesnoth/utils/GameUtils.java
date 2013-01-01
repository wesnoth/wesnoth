/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.ui.console.MessageConsole;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.projects.ProjectUtils;

/**
 * Utilities class for handling with the Wesnoth Game
 */
public class GameUtils
{
    /**
     * Runs campaign from the selected project
     */
    public static void runCampaign( )
    {
        Thread gameThread = new Thread( new Runnable( ) {
            @Override
            public void run( )
            {
                runCampaignScenario( false );
            }
        } );
        gameThread.start( );
    }

    /**
     * Runs a scenario from the selected file
     */
    public static void runScenario( )
    {
        Thread gameThread = new Thread( new Runnable( ) {
            @Override
            public void run( )
            {
                runCampaignScenario( true );
            }
        } );
        gameThread.start( );
    }

    protected static void runCampaignScenario( boolean scenario )
    {
        IResource selectedResource = WorkspaceUtils.getSelectedResource( );

        if( selectedResource == null ) {
            GUIUtils.showWarnMessageBox( Messages.GameUtils_0 + "" ); //$NON-NLS-1$
            return;
        }

        try {
            String campaignId = null;
            String scenarioId = null;

            campaignId = ProjectUtils.getCacheForProject(
                selectedResource.getProject( ) ).getWMLConfig( "_main.cfg" ).CampaignId; //$NON-NLS-1$

            if( scenario == true && selectedResource instanceof IFile ) {
                scenarioId = ProjectUtils.getCacheForProject(
                    selectedResource.getProject( ) ).getWMLConfig(
                    selectedResource.getProjectRelativePath( ).toString( ) ).ScenarioId;
            }

            if( campaignId == null ) {
                GUIUtils.showErrorMessageBox( Messages.GameUtils_2 + "" ); //$NON-NLS-1$
                return;
            }

            if( scenario == true && scenarioId == null ) {
                GUIUtils.showErrorMessageBox( Messages.GameUtils_4 + "" ); //$NON-NLS-1$
                return;
            }

            List< String > args = new ArrayList< String >( );

            // --campaign
            args.add( "-c" ); //$NON-NLS-1$
            args.add( campaignId );

            if( scenario == true ) {
                args.add( "--campaign-scenario" ); //$NON-NLS-1$
                args.add( scenarioId );
            }

            startGame(
                WesnothInstallsUtils
                    .getInstallNameForResource( selectedResource ),
                args );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Starts the game
     */
    public static void startGame( )
    {
        IResource selectedRes = WorkspaceUtils.getSelectedResource( );
        if( selectedRes == null ) {
            startGame( null, null );
        }
        else {
            WesnothInstallsUtils.setupInstallForResource( selectedRes );
            startGame(
                WesnothInstallsUtils
                    .getInstallNameForResource( selectedRes ),
                null );
        }
    }

    /**
     * Starts the wesnoth game with the specified extraArguments
     * 
     * @param installName
     *        The install name to use when launching the game
     * 
     * @param extraArgs
     *        Extra arguments given to the game, or null.
     */
    public static void startGame( String installName, List< String > extraArgs )
    {
        List< String > args = new ArrayList< String >( );

        Paths paths = Preferences.getPaths( installName );
        String wesnothExec = paths.getWesnothExecutablePath( );
        String workingDir = paths.getWorkingDir( );

        if( wesnothExec.isEmpty( ) || workingDir.isEmpty( ) ) {
            GUIUtils.showErrorMessageBox( Messages.GameUtils_7 );
            return;
        }

        if( extraArgs != null ) {
            args.addAll( extraArgs );
        }

        // add the user's data directory path
        args.add( "--config-dir" ); //$NON-NLS-1$
        args.add( paths.getUserDir( ) );

        // we need to add the working dir (backward compatibility)
        args.add( workingDir );

        MessageConsole console = GUIUtils.createConsole( Messages.GameUtils_9,
            null, true );
        ExternalToolInvoker.launchTool( wesnothExec, args,
            new OutputStream[] { console.newMessageStream( ) },
            new OutputStream[] { console.newMessageStream( ) } );
    }

    /**
     * Starts editor
     */
    public static void startEditor( )
    {
        startEditor( "" ); //$NON-NLS-1$
    }

    /**
     * Starts the game editor on the specified file
     * 
     * @param file
     *        The file to be edited
     */
    public static void startEditor( IFile file )
    {
        if( file == null || ! file.exists( ) ) {
            Logger.getInstance( ).log( "non-existing map file", //$NON-NLS-1$
                Messages.GameUtils_12 );
            return;
        }

        startEditor( file.getLocation( ).toOSString( ) );
    }

    /**
     * Starts the editor
     * 
     * @param mapName
     */
    public static void startEditor( String mapName )
    {
        startGame( WesnothInstallsUtils.getInstallNameForResource( mapName ),
            getEditorLaunchArguments( mapName ) );
    }

    /**
     * Gets a list of parameters for the game editor
     * 
     * @param mapName
     *        the map to launch
     * @return A list of string with the editor launch arguments.
     */
    public static List< String > getEditorLaunchArguments( String mapName )
    {

        List< String > args = new ArrayList< String >( 3 );

        args.add( "-e" ); //$NON-NLS-1$
        if( mapName != null && ! ( mapName.isEmpty( ) ) ) {
            args.add( mapName );
        }

        return args;
    }
}

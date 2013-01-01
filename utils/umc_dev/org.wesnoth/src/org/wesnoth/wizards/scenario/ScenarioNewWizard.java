/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.scenario;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Map;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.WorkspaceUtils;
import org.wesnoth.wizards.WizardTemplate;

/**
 * This is a sample new wizard. Its role is to create a new file resource in the
 * provided container. If the container resource (a folder or a project) is
 * selected in the workspace when the wizard is opened, it will accept it as the
 * target container. The wizard creates one file with the extension "cfg". If a
 * sample multi-page editor (also available as a template) is registered for the
 * same extension, it will be able to open it.
 */
public class ScenarioNewWizard extends WizardTemplate
{
    private ScenarioPage0 page0_;
    private ScenarioPage1 page1_;
    private ScenarioPage2 page2_;

    /**
     * Constructor for ScenarioNewWizard.
     */
    public ScenarioNewWizard( )
    {
        super( );
        setWindowTitle( Messages.ScenarioNewWizard_0 );
        setNeedsProgressMonitor( true );
    }

    /**
     * Adding the page to the wizard.
     */
    @Override
    public void addPages( )
    {
        page0_ = new ScenarioPage0( selectionContainer_ );
        addPage( page0_ );

        if( selectionContainer_ != null ) {
            Map< String, String > props = ProjectUtils
                .getPropertiesForProject( selectionContainer_.getProject( ) );
            if( props.get( "difficulties" ) != null ) //$NON-NLS-1$
            {
                page1_ = new ScenarioPage1( );
                addPage( page1_ );
            }
        }

        page2_ = new ScenarioPage2( );
        addPage( page2_ );

        super.addPages( );
    }

    /**
     * This method is called when 'Finish' button is pressed in the wizard. We
     * will create an operation and run it using wizard as execution context.
     */
    @Override
    public boolean performFinish( )
    {
        final String containerName = page0_.getProjectName( );
        final String fileName = page0_.getFileName( );
        IRunnableWithProgress op = new IRunnableWithProgress( ) {
            @Override
            public void run( IProgressMonitor monitor )
                throws InvocationTargetException
            {
                try {
                    doFinish( containerName, fileName, monitor );
                } catch( Exception e ) {
                    throw new InvocationTargetException( e );
                } finally {
                    monitor.done( );
                }
            }
        };
        try {
            getContainer( ).run( false, false, op );
        } catch( InterruptedException e ) {
            return false;
        } catch( InvocationTargetException e ) {
            Logger.getInstance( ).logException( e );
            return false;
        }
        return true;
    }

    /**
     * The worker method. It will find the container, create the file if missing
     * or just replace its contents, and open the editor on the newly created
     * file.
     */
    private void doFinish( String containerName, String fileName,
        IProgressMonitor monitor ) throws Exception
    {
        monitor.beginTask( Messages.ScenarioNewWizard_2 + fileName, 2 );
        IWorkspaceRoot root = ResourcesPlugin.getWorkspace( ).getRoot( );
        IResource resource = root.findMember( new Path( containerName ) );

        if( ! resource.exists( ) || ! ( resource instanceof IContainer ) ) {
            throw new Exception(
                "Container " + containerName + " does not exist." ); //$NON-NLS-1$ //$NON-NLS-2$
        }

        final IContainer container = ( IContainer ) resource;
        final IFile file = container.getFile( new Path( fileName ) );

        InputStream stream = getScenarioStream( container );

        try {
            if( stream == null ) {
                return;
            }

            if( file.exists( ) ) {
                file.setContents( stream, true, true, monitor );
            }
            else {
                file.create( stream, true, monitor );
            }

            stream.close( );
            container.refreshLocal( IResource.DEPTH_INFINITE,
                new NullProgressMonitor( ) );
        } catch( IOException e ) {
            Logger.getInstance( ).logException( e );
        }

        monitor.worked( 1 );
        monitor.setTaskName( Messages.ScenarioNewWizard_5 );
        getShell( ).getDisplay( ).asyncExec( new Runnable( ) {
            @Override
            public void run( )
            {
                IWorkbenchPage page = PlatformUI.getWorkbench( )
                    .getActiveWorkbenchWindow( ).getActivePage( );
                try {
                    IDE.openEditor( page, file, true );
                } catch( PartInitException e ) {
                }
            }
        } );
        monitor.worked( 1 );
        monitor.done( );
    }

    /**
     * Returns the scenario file contents as an InputStream
     * 
     * @throws Exception
     */
    private InputStream getScenarioStream( IContainer container )
        throws Exception
    {
        ArrayList< ReplaceableParameter > params = new ArrayList< ReplaceableParameter >( );

        // common variables (sp + mp)
        params.add( new ReplaceableParameter(
            "$$scenario_id", page0_.getScenarioId( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$next_scenario_id", page0_.getNextScenarioId( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$scenario_name", page0_.getScenarioName( ) ) ); //$NON-NLS-1$

        String mapData = ""; //$NON-NLS-1$
        if( ! page0_.getMapData( ).isEmpty( ) ) {
            String userMapPath = page0_.getMapData( ).replace( "~add-ons", //$NON-NLS-1$
                container.getParent( ).getLocation( ).toOSString( ) + "/" ); //$NON-NLS-1$
            // trim the '{' and '}'
            userMapPath = userMapPath.substring( 1, userMapPath.length( ) - 1 );

            if( ! page0_.getIsMapEmbedded( ) ) {
                mapData = page0_.getMapData( );
                ResourceUtils.copyTo( new File( page0_.getRawMapPath( ) ),
                    new File( userMapPath ) );
            }
            else {
                mapData = ResourceUtils
                    .getFileContents( new File( userMapPath ) );
            }
        }
        params.add( new ReplaceableParameter( "$$map_data", mapData ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$turns_number", String.valueOf( page0_.getTurnsNumber( ) ) ) ); //$NON-NLS-1$

        String startingGold = ""; //$NON-NLS-1$
        if( page1_ != null ) {
            startingGold = page1_.getStartingGoldByDifficulties( );
            if( startingGold == null ) {
                throw new Exception( "incorrect arguments" ); //$NON-NLS-1$
            }
        }
        params
            .add( new ReplaceableParameter( "$$starting_gold", startingGold ) ); //$NON-NLS-1$

        // multiplayer only variables
        params.add( new ReplaceableParameter(
            "$$allow_new_game", page2_.getAllowNewGame( ) ) ); //$NON-NLS-1$

        String template = TemplateProvider
            .getInstance( )
            .getProcessedTemplate(
                page2_.isMultiplayerScenario( ) ? "multiplayer": "scenario", params ); //$NON-NLS-1$ //$NON-NLS-2$

        if( template == null ) {
            GUIUtils.showMessageBox( WorkspaceUtils.getWorkbenchWindow( ),
                Messages.ScenarioNewWizard_20 );
            return null;
        }

        return new ByteArrayInputStream( template.getBytes( ) );
    }
}

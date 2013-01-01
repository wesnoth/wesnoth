/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.era;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.wizards.WizardTemplate;

/**
 * Wizard for creating a new Era
 */
public class EraNewWizard extends WizardTemplate
{
    private EraPage0 page0_;

    /**
     * Creates a new {@link EraNewWizard}
     */
    public EraNewWizard( )
    {
        setWindowTitle( Messages.EraNewWizard_0 );
        setNeedsProgressMonitor( true );
    }

    @Override
    public void addPages( )
    {
        page0_ = new EraPage0( );
        addPage( page0_ );

        super.addPages( );
    }

    @Override
    public boolean performFinish( )
    {
        final String containerName = page0_.getDirectoryName( );
        final String fileName = page0_.getFileName( );
        IRunnableWithProgress op = new IRunnableWithProgress( ) {
            @Override
            public void run( IProgressMonitor monitor )
            {
                try {
                    doFinish( containerName, fileName, monitor );
                } catch( CoreException e ) {
                    Logger.getInstance( ).logException( e );
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
        }
        return true;
    }

    private void doFinish( String containerName, String fileName,
        IProgressMonitor monitor ) throws CoreException
    {
        // create a sample file
        monitor.beginTask( Messages.EraNewWizard_1 + fileName, 10 );
        IWorkspaceRoot root = ResourcesPlugin.getWorkspace( ).getRoot( );
        IResource resource = root.findMember( new Path( containerName ) );

        IContainer container = ( IContainer ) resource;
        final IFile file = container.getFile( new Path( fileName ) );

        try {
            InputStream stream = getEraStream( );

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
        } catch( IOException e ) {
            Logger.getInstance( ).logException( e );
        }

        monitor.worked( 5 );
        monitor.setTaskName( Messages.EraNewWizard_2 );
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
        monitor.worked( 5 );
        monitor.done( );
    }

    private InputStream getEraStream( )
    {
        ArrayList< ReplaceableParameter > params = new ArrayList< ReplaceableParameter >( );

        params.add( new ReplaceableParameter( "$$era_id", page0_.getEraID( ) ) ); //$NON-NLS-1$
        params
            .add( new ReplaceableParameter( "$$era_name", page0_.getEraName( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$require_era", String.valueOf( page0_.getRequiresEra( ) ) ) ); //$NON-NLS-1$

        String template = TemplateProvider.getInstance( ).getProcessedTemplate(
            "era", params ); //$NON-NLS-1$

        if( template == null ) {
            Logger.getInstance( ).log( "'era' template not found", //$NON-NLS-1$
                Messages.EraNewWizard_8 );
            return null;
        }

        return new ByteArrayInputStream( template.getBytes( ) );
    }
}

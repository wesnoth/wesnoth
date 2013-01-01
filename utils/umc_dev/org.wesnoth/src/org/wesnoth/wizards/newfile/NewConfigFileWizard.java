/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.newfile;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.wizards.WizardTemplate;

/**
 * Wizard for creating new config files
 */
public class NewConfigFileWizard extends WizardTemplate
{
    private NewConfigFilePage0 page_;

    /**
     * Constructor for NewConfigFileWizard.
     */
    public NewConfigFileWizard( )
    {
        super( );
        setNeedsProgressMonitor( true );
    }

    /**
     * Adding the page to the wizard.
     */
    @Override
    public void addPages( )
    {
        page_ = new NewConfigFilePage0( selection_ );
        addPage( page_ );

        super.addPages( );
    }

    /**
     * This method is called when 'Finish' button is pressed in
     * the wizard. We will create an operation and run it
     * using wizard as execution context.
     */
    @Override
    public boolean performFinish( )
    {
        final String containerName = page_.getContainerName( );
        final String fileName = page_.getFileName( );
        IRunnableWithProgress op = new IRunnableWithProgress( ) {
            @Override
            public void run( IProgressMonitor monitor )
            {
                try {
                    doFinish( containerName, fileName, monitor );
                } catch( Exception e ) {
                    Logger.getInstance( ).logException( e );
                } finally {
                    monitor.done( );
                }
            }
        };
        try {
            getContainer( ).run( true, false, op );
        } catch( InterruptedException e ) {
            return false;
        } catch( InvocationTargetException e ) {
            Logger.getInstance( ).logException( e );
        }
        return true;
    }

    /**
     * The worker method. It will find the container, create the
     * file if missing or just replace its contents, and open
     * the editor on the newly created file.
     */
    private void doFinish( String containerName, String fileName,
        IProgressMonitor monitor ) throws Exception
    {
        // create the file
        monitor.beginTask( Messages.NewConfigFileWizard_0 + fileName, 2 );
        IWorkspaceRoot root = ResourcesPlugin.getWorkspace( ).getRoot( );
        IResource resource = root.findMember( new Path( containerName ) );

        if( ! resource.exists( ) || ! ( resource instanceof IContainer ) ) {
            throw new Exception(
                "Container " + containerName + " does not exist." ); //$NON-NLS-1$ //$NON-NLS-2$
        }

        IContainer container = ( IContainer ) resource;
        final IFile file = container.getFile( new Path( fileName ) );
        try {
            InputStream stream = new ByteArrayInputStream( "".getBytes( ) ); //$NON-NLS-1$
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
        monitor.worked( 1 );
        monitor.setTaskName( Messages.NewConfigFileWizard_4 );
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
    }
}

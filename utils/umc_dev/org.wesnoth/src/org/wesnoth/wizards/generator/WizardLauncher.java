/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import java.io.ByteArrayInputStream;
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

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.utils.EditorUtils;
import org.wesnoth.utils.WizardUtils;
import org.wesnoth.wizards.WizardTemplate;

/**
 * A Wizard that launches a custom wizard for selected tag
 */
public class WizardLauncher extends WizardTemplate
{
    WizardLauncherPage0 page0_;
    WizardLauncherPage1 page1_;
    WizardGenerator     wizard_;

    /**
     * Creates a new {@link WizardLauncher}
     */
    public WizardLauncher( )
    {
        setWindowTitle( Messages.WizardLauncher_0 );
        setNeedsProgressMonitor( true );
    }

    @Override
    public void addPages( )
    {
        page0_ = new WizardLauncherPage0( );
        addPage( page0_ );

        page1_ = new WizardLauncherPage1( );
        addPage( page1_ );

        super.addPages( );
    }

    @Override
    public boolean performFinish( )
    {
        wizard_ = new WizardGenerator( page1_.getTagDescription( )
            + Messages.WizardLauncher_1, page1_.getTagName( ), ( byte ) 0 );
        WizardUtils.launchWizard( wizard_, getShell( ), selection_ );
        if( ! wizard_.isFinished( ) ) {
            return false;
        }

        IRunnableWithProgress op = new IRunnableWithProgress( ) {
            @Override
            public void run( IProgressMonitor monitor )
                throws InvocationTargetException
            {
                doFinish( monitor );
                monitor.done( );
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

    private void doFinish( IProgressMonitor monitor )
    {
        try {
            // The file is opened in the editor -> just copy-paste the text
            if( ! ( page0_.getIsTargetNewFile( ) ) ) {
                EditorUtils.writeInEditor( EditorUtils.getEditedFile( ),
                    wizard_.getData( ).toString( ) );
                return;
            }

            final String containerName = page0_.getDirectoryName( );
            final String fileName = page0_.getFileName( );

            // create the file
            monitor.beginTask( Messages.WizardLauncher_2 + fileName, 10 );
            IWorkspaceRoot root = ResourcesPlugin.getWorkspace( ).getRoot( );
            IResource resource = root.findMember( new Path( containerName ) );

            IContainer container = ( IContainer ) resource;
            final IFile file = container.getFile( new Path( fileName ) );

            InputStream stream = new ByteArrayInputStream( wizard_.getData( )
                .toString( ).getBytes( ) );

            if( file.exists( ) ) {
                file.setContents( stream, true, true, monitor );
            }
            else {
                file.create( stream, true, monitor );
            }

            stream.close( );

            monitor.worked( 5 );
            monitor.setTaskName( Messages.WizardLauncher_3 );
            getShell( ).getDisplay( ).asyncExec( new Runnable( ) {
                @Override
                public void run( )
                {
                    EditorUtils.openEditor( file, true );
                }
            } );
            monitor.worked( 5 );

            monitor.done( );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
        }
    }
}

/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.emptyproject;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.operation.IRunnableWithProgress;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.wizards.WizardProjectPageTemplate;
import org.wesnoth.wizards.WizardTemplate;

/**
 * Wizard for creating a new empty project
 */
public class EmptyProjectNewWizard extends WizardTemplate
{
    protected WizardProjectPageTemplate page0_;
    protected EmptyProjectPage1         page1_;

    /**
     * Creates a new {@link EmptyProjectNewWizard}
     */
    public EmptyProjectNewWizard( )
    {
        setWindowTitle( Messages.EmptyProjectNewWizard_0 );
    }

    @Override
    public void addPages( )
    {
        page0_ = new WizardProjectPageTemplate( "emptyProjectPage0",
            Messages.EmptyProjectPage0_1, Messages.EmptyProjectPage0_2 );
        addPage( page0_ );

        page1_ = new EmptyProjectPage1( );
        addPage( page1_ );
    }

    @Override
    public boolean canFinish( )
    {
        // pbl information is not necessary
        return page0_.isPageComplete( );
    }

    @Override
    public boolean performFinish( )
    {
        try {
            // let's create the project
            getContainer( ).run( false, false, new IRunnableWithProgress( ) {
                @Override
                public void run( IProgressMonitor monitor )
                    throws InvocationTargetException, InterruptedException
                {
                    page0_.createProject( monitor, "empty_project",
                        getParameters( ), page1_.getGeneratePBLFile( ) );
                    monitor.done( );
                }
            } );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
            return false;
        }
        return true;
    }

    private List< ReplaceableParameter > getParameters( )
    {
        List< ReplaceableParameter > params = new ArrayList< ReplaceableParameter >( );

        params.add( new ReplaceableParameter(
            "$$campaign_name", page1_.getCampaignName( ) ) ); //$NON-NLS-1$
        params
            .add( new ReplaceableParameter( "$$author", page1_.getAuthor( ) ) ); //$NON-NLS-1$
        params
            .add( new ReplaceableParameter( "$$version", page1_.getVersion( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$description", page1_.getPBLDescription( ) ) ); //$NON-NLS-1$
        params
            .add( new ReplaceableParameter( "$$icon", page1_.getIconPath( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter( "$$email", page1_.getEmail( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$passphrase", page1_.getPassphrase( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$translations_dir", page1_.getTranslationDir( ) ) ); //$NON-NLS-1$

        params.add( new ReplaceableParameter(
            "$$project_name", page0_.getProjectName( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$project_dir_name", page0_.getProjectName( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter( "$$type", page1_.getType( ) ) ); //$NON-NLS-1$

        return params;
    }
}

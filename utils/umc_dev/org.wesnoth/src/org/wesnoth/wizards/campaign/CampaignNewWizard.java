/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.campaign;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.operation.IRunnableWithProgress;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.wizards.WizardProjectPageTemplate;
import org.wesnoth.wizards.WizardTemplate;

/**
 * Wizard for creating a new campaign
 */
public class CampaignNewWizard extends WizardTemplate
{
    protected WizardProjectPageTemplate page0_;
    protected CampaignPage1             page1_;
    protected CampaignPage2             page2_;

    /**
     * Creates a new {@link CampaignNewWizard}
     */
    public CampaignNewWizard( )
    {
        setWindowTitle( Messages.CampaignNewWizard_0 );
        setNeedsProgressMonitor( true );
    }

    @Override
    public void addPages( )
    {
        page0_ = new WizardProjectPageTemplate( "campaignPage0",
            Messages.CampaignPage0_1, Messages.CampaignPage0_2 );
        addPage( page0_ );

        page1_ = new CampaignPage1( );
        addPage( page1_ );

        page2_ = new CampaignPage2( );
        addPage( page2_ );
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
                    IProject currentProject = page0_.createProject( monitor,
                        "campaign_structure", getParameters( ),
                        page1_.needsPBLFile( ) );

                    // store some campaign-related info
                    ProjectUtils.getPropertiesForProject( currentProject ).put(
                        "difficulties", page2_.getDifficulties( ) ); //$NON-NLS-1$
                    ProjectUtils.getCacheForProject( currentProject )
                        .saveCache( );

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
            "$$description", page1_.getCampaignDescription( ) ) ); //$NON-NLS-1$
        params
            .add( new ReplaceableParameter( "$$icon", page1_.getIconPath( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter( "$$email", page1_.getEmail( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$passphrase", page1_.getPassphrase( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$translations_dir", page1_.getTranslationDir( ) ) ); //$NON-NLS-1$

        params.add( new ReplaceableParameter(
            "$$campaign_id", page2_.getCampaignId( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter( "$$abrev", page2_.getAbbrev( ) ) ); //$NON-NLS-1$
        params
            .add( new ReplaceableParameter( "$$define", page2_.getDefine( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$difficulties", page2_.getDifficulties( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$first_scenario", page2_.getFirstScenario( ) ) ); //$NON-NLS-1$

        params.add( new ReplaceableParameter(
            "$$project_name", page0_.getProjectName( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$project_dir_name", page0_.getProjectName( ) ) ); //$NON-NLS-1$
        params.add( new ReplaceableParameter(
            "$$type", page1_.isMultiplayer( ) ? "campaign_mp": "campaign" ) ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$

        return params;
    }
}

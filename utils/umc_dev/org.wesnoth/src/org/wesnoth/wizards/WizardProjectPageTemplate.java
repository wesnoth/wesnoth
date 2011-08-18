/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards;

import java.util.List;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;

import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.Pair;
import org.wesnoth.utils.ResourceUtils;

/**
 * A page that creates a new project and associates it to a specific
 * install
 */
public class WizardProjectPageTemplate extends WizardNewProjectCreationPage
{
    private Combo cmbInstalls_;

    /**
     * Creates a new project creation wizard page
     * 
     * @param pageName
     *        The name of the page
     * @param title
     *        The title of the page
     * @param message
     *        The message showed in the page
     */
    public WizardProjectPageTemplate( String pageName, String title,
        String message )
    {
        super( pageName );

        setTitle( title );
        setMessage( message );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite composite = new Composite( ( Composite ) getControl( ),
            SWT.NULL );
        composite.setLayout( new GridLayout( 2, false ) );

        Label lblWesnothInstall = new Label( composite, SWT.NONE );
        lblWesnothInstall
            .setToolTipText( "Select the wesnoth install this project corresponds to." );
        lblWesnothInstall.setLayoutData( new GridData( SWT.RIGHT, SWT.CENTER,
            false, false, 1, 1 ) );
        lblWesnothInstall.setText( "Wesnoth Install:" );

        cmbInstalls_ = new Combo( composite, SWT.READ_ONLY );
        GridData gd_cmbInstalls = new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 );
        gd_cmbInstalls.widthHint = 154;
        cmbInstalls_.setLayoutData( gd_cmbInstalls );

        WesnothInstallsUtils.fillComboWithInstalls( cmbInstalls_ );
    }

    /**
     * Returns true if the project needs a {@code build.xml} file.
     * 
     * @return Returns true if the project needs a {@code build.xml} file.
     */
    public boolean needsBuildXML( )
    {
        Paths paths = Preferences.getPaths( getSelectedInstallName( ) );
        String projectPath = getProjectHandle( ).getLocation( ).toOSString( );
        return( ! ResourceUtils.isCampaignDirPath( paths, projectPath ) && ! ResourceUtils
            .isUserAddonsDirPath( paths, projectPath ) );
    }

    /**
     * Returns the selected install
     * 
     * @return The selected install name string.
     */
    public String getSelectedInstallName( )
    {
        return cmbInstalls_.getText( );
    }

    /**
     * Creates the project this page was setup with
     * 
     * @param monitor
     *        The monitor for monitoring progress
     * @param templateName
     *        The name of the template of the created project
     * @param params
     *        The parameters to use when expanding the project template
     * @param generatePBL
     *        True to generate the pbl file in the project
     * 
     * @return The newly created project's handle
     */
    public IProject createProject( IProgressMonitor monitor,
        String templateName, List< ReplaceableParameter > params,
        boolean generatePBL )
    {
        monitor.subTask( "Creating the project structure" );

        IProject currentProject = ProjectUtils.createWesnothProject(
            getProjectName( ), getLocationPath( ).toOSString( ),
            getSelectedInstallName( ), monitor );
        monitor.worked( 2 );

        String projectTemplate = TemplateProvider.getInstance( )
            .getProcessedTemplate( templateName, params );

        List< Pair< String, String >> files;
        List< String > dirs;
        Pair< List< Pair< String, String >>, List< String >> tmp = TemplateProvider
            .getInstance( ).getFilesDirectories( projectTemplate );
        files = tmp.First;
        dirs = tmp.Second;

        for( Pair< String, String > file: files ) {
            if( file.Second.equals( "pbl" ) && //$NON-NLS-1$
                ! generatePBL ) {
                continue;
            }

            if( file.Second.equals( "build_xml" ) && //$NON-NLS-1$
                ! needsBuildXML( ) ) {
                continue;
            }

            ResourceUtils.createFile(
                currentProject,
                file.First,
                TemplateProvider.getInstance( ).getProcessedTemplate(
                    file.Second, params ), true );
            monitor.worked( 1 );
        }

        for( String dir: dirs ) {
            ResourceUtils.createFolder( currentProject, dir );
            monitor.worked( 1 );
        }

        monitor.done( );
        return currentProject;
    }
}

/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.utils.ResourceUtils;

/**
 * A page that creates a new project and associates it to a specific
 * install
 */
public class WizardProjectPageTemplate extends WizardNewProjectCreationPage
{
    private Combo cmbInstalls_;

    /** {@inheritDoc}
     */
    public WizardProjectPageTemplate( String pageName,
            String title, String message)
    {
        super( pageName );

        setTitle( title );
        setMessage( message );
    }

    @Override
    public void createControl(Composite parent)
    {
        super.createControl( parent );
        Composite composite = new Composite( ( Composite ) getControl( ), SWT.NULL );
        composite.setLayout(new GridLayout(2, false));

        Label lblWesnothInstall = new Label(composite, SWT.NONE);
        lblWesnothInstall.setToolTipText("Select the wesnoth install this project corresponds to.");
        lblWesnothInstall.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
        lblWesnothInstall.setText("Wesnoth Install:");

        cmbInstalls_ = new Combo(composite, SWT.NONE);
        GridData gd_cmbInstalls = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
        gd_cmbInstalls.widthHint = 154;
        cmbInstalls_.setLayoutData(gd_cmbInstalls);
    }

    /**
     * Returns true if the project needs a {@code build.xml} file.
     * @return Returns true if the project needs a {@code build.xml} file.
     */
    public boolean needsBuildXML()
    {
        Paths paths = Preferences.getPaths( getSelectedInstallName() );
        String projectPath = getProjectHandle( ).getLocation( ).toOSString( );
        return ( ! ResourceUtils.isCampaignDirPath( paths, projectPath ) &&
                 ! ResourceUtils.isUserAddonsDirPath( paths, projectPath ) );
    }

    /**
     * Returns the selected install
     * @return
     */
    public String getSelectedInstallName( )
    {
        return cmbInstalls_.getText( );
    }
}

/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.installs;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;

/**
 * A simple dialog that asks the user to select between existing
 * Wesnoth installs
 */
public class SelectWesnothInstallDialog extends Dialog
{
    private Combo cmbInstall_;

    /**
     * Creates a new dialog on the specified shell
     * 
     * @param parentShell
     *        The shell to create the dialog in
     */
    public SelectWesnothInstallDialog( Shell parentShell )
    {
        super( parentShell );

        cmbInstall_ = null;
    }

    @Override
    protected void configureShell( Shell newShell )
    {
        super.configureShell( newShell );

        newShell.setText( "Select the Wesnoth Install" );
    }

    @Override
    protected Control createDialogArea( Composite parent )
    {
        Composite composite = new Composite( parent, SWT.NONE );
        composite.setLayout( new GridLayout( 2, false ) );

        Label lblWesnothInstall = new Label( composite, SWT.NONE );
        lblWesnothInstall.setLayoutData( new GridData( SWT.RIGHT, SWT.CENTER,
            false, false, 1, 1 ) );
        lblWesnothInstall.setText( "Wesnoth Install:" );

        cmbInstall_ = new Combo( composite, SWT.READ_ONLY );
        GridData gd_cmbInstall_ = new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 );
        gd_cmbInstall_.widthHint = 163;
        cmbInstall_.setLayoutData( gd_cmbInstall_ );

        WesnothInstallsUtils.fillComboWithInstalls( cmbInstall_ );

        return super.createDialogArea( parent );
    }

    @Override
    protected Point getInitialSize( )
    {
        return new Point( 291, 123 );
    }

    /**
     * Gets the install selected by the user
     * 
     * @return A string with the name of the install selected
     */
    public String getSelectedInstallName( )
    {
        if( cmbInstall_ == null ) {
            return "";
        }

        return cmbInstall_.getText( );
    }
}

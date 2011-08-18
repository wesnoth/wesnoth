package org.wesnoth.propertypages;

import java.util.List;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.ui.dialogs.PropertyPage;

import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstall;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;

/**
 * A PropertyPage for Wesnoth Projects
 */
public class WesnothProjectPage extends PropertyPage
{
    private Combo        cmbInstall_;
    private ProjectCache currentProjectCache_;
    private String       currentInstall_;

    /**
     * Creates a new {@link WesnothProjectPage}
     */
    public WesnothProjectPage( )
    {
        currentProjectCache_ = null;
        cmbInstall_ = null;
        currentInstall_ = null;
    }

    @Override
    protected Control createContents( Composite parent )
    {
        IAdaptable selectedElement = getElement( );
        if( ! ( selectedElement instanceof IProject ) ) {
            return new Composite( parent, 0 );
        }

        IProject selectedProject = ( IProject ) selectedElement;

        currentProjectCache_ = ProjectUtils
            .getCacheForProject( selectedProject );

        Composite newComposite = new Composite( parent, 0 );
        newComposite.setLayout( new FillLayout( SWT.VERTICAL ) );

        Group grpGeneral = new Group( newComposite, SWT.NONE );
        grpGeneral.setText( Messages.WesnothProjectPage_0 );
        grpGeneral.setLayout( new GridLayout( 2, false ) );

        Label lblNewLabel = new Label( grpGeneral, SWT.NONE );
        lblNewLabel.setLayoutData( new GridData( SWT.RIGHT, SWT.CENTER, false,
            false, 1, 1 ) );
        lblNewLabel.setText( Messages.WesnothProjectPage_1 );

        cmbInstall_ = new Combo( grpGeneral, SWT.READ_ONLY );
        cmbInstall_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );

        // fill the installs
        List< WesnothInstall > installs = WesnothInstallsUtils.getInstalls( );

        boolean foundInstallInList = false;
        String installName = currentProjectCache_.getInstallName( );

        for( WesnothInstall wesnothInstall: installs ) {
            cmbInstall_.add( wesnothInstall.getName( ) );

            // current install is default?
            if( wesnothInstall.getName( ).equalsIgnoreCase( installName ) ) {
                cmbInstall_.select( cmbInstall_.getItemCount( ) - 1 );
                currentInstall_ = wesnothInstall.getName( );
                foundInstallInList = true;
            }
        }

        // check if the current install name is existing
        if( ! foundInstallInList ) {
            setErrorMessage( Messages.WesnothProjectPage_2 );
        }

        return newComposite;
    }

    @Override
    public boolean performOk( )
    {
        String selectedInstall = cmbInstall_.getText( );
        // save settings.
        if( currentProjectCache_ != null && ! selectedInstall.isEmpty( ) ) {
            currentProjectCache_.setInstallName( selectedInstall );

            if( ! selectedInstall.equals( currentInstall_ ) ) {
                // relink the data directory
                ProjectUtils.createCoreLibraryFolder( currentProjectCache_
                    .getProject( ), IResource.BACKGROUND_REFRESH );
            }
        }

        return true;
    }
}

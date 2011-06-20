package org.wesnoth.propertypages;

import java.util.List;

import org.eclipse.core.resources.IProject;
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
import org.eclipse.ui.IWorkbenchPropertyPage;
import org.eclipse.ui.dialogs.PropertyPage;
import org.wesnoth.utils.ProjectCache;
import org.wesnoth.utils.ProjectUtils;
import org.wesnoth.utils.WesnothInstallsUtils;
import org.wesnoth.utils.WesnothInstallsUtils.WesnothInstall;

public class WesnothProjectPage extends PropertyPage implements IWorkbenchPropertyPage
{
    private Combo cmbInstall_;
    private ProjectCache currProjectCache_;

    public WesnothProjectPage()
    {
        currProjectCache_ = null;
    }

    @Override
    protected Control createContents( Composite parent )
    {
        IAdaptable selectedElement = getElement( );
        if ( !( selectedElement instanceof IProject ) ) {
            return new Composite( parent, 0 );
        }

        IProject selectedProject = (IProject) selectedElement;

        currProjectCache_ = ProjectUtils.getCacheForProject( selectedProject );

        Composite newComposite = new Composite( parent, 0 );
        newComposite.setLayout(new FillLayout(SWT.VERTICAL));

        Group grpGeneral = new Group(newComposite, SWT.NONE);
        grpGeneral.setText("General");
        grpGeneral.setLayout(new GridLayout(2, false));

        Label lblNewLabel = new Label(grpGeneral, SWT.NONE);
        lblNewLabel.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
        lblNewLabel.setText("Wesnoth install:");

        cmbInstall_ = new Combo(grpGeneral, SWT.READ_ONLY );
        cmbInstall_.setItems(new String[] {"Default"});
        cmbInstall_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
        cmbInstall_.select(0);

        // fill the installs
        List<WesnothInstall> installs = WesnothInstallsUtils.getInstalls( );

        boolean foundInstallInList = false;
        String installName = currProjectCache_.getInstallName( );

        for ( WesnothInstall wesnothInstall : installs )
        {
            if ( wesnothInstall.Name.equalsIgnoreCase( "Default" ) )
                 continue;

            cmbInstall_.add( wesnothInstall.Name );

            // current install is default?
            if ( wesnothInstall.Name.equalsIgnoreCase( installName ) )
            {
                cmbInstall_.select( cmbInstall_.getItemCount( ) - 1 );
                foundInstallInList = true;
            }
        }

        // check if the current install name is existing
        if ( !foundInstallInList && !installName.equalsIgnoreCase( "default" ) ) {
            setErrorMessage( "The currently selected install does not exist. Please select another." );
        }

        return newComposite;
    }

    @Override
    public boolean performOk()
    {
        // save settings.
        if ( currProjectCache_ != null ){
            currProjectCache_.setInstallName( cmbInstall_.getText( ) );
        }

        return true;
    }
}

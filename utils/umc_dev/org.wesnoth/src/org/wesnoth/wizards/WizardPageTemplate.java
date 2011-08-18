/**
 * @author Timotei Dolean
 */
package org.wesnoth.wizards;

import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;

/**
 * A template wizard page with some default functionality
 */
public class WizardPageTemplate extends WizardPage
{
    protected WizardPageTemplate( String pageName )
    {
        super( pageName );
    }

    @Override
    public void createControl( Composite parent )
    {
        WesnothPlugin.getDefault( ).getWorkbench( ).getHelpSystem( )
            .setHelp( getShell( ), "org.wesnoth.wizardHelp" ); //$NON-NLS-1$
    }

    @Override
    public WizardTemplate getWizard( )
    {
        return ( WizardTemplate ) super.getWizard( );
    }

    /**
     * Uses the standard container selection dialog to choose the new value for
     * the project field.
     * 
     * @return An {@link IPath} instance.
     */
    public IPath handleBrowseContainer( )
    {
        ContainerSelectionDialog dialog = new ContainerSelectionDialog(
            getShell( ), ResourcesPlugin.getWorkspace( ).getRoot( ), false,
            Messages.NewWizardPageTemplate_1 );
        if( dialog.open( ) == Window.OK ) {
            Object[] result = dialog.getResult( );
            if( result.length == 1 ) {
                try {
                    getWizard( ).selectionContainer_ = ResourcesPlugin
                        .getWorkspace( ).getRoot( )
                        .getFolder( ( Path ) result[0] );
                } catch( IllegalArgumentException e ) {
                    // the path is a project
                    getWizard( ).selectionContainer_ = ResourcesPlugin
                        .getWorkspace( ).getRoot( )
                        .getProject( result[0].toString( ) );
                }
                return ( Path ) result[0];
            }
        }
        return null;
    }
}

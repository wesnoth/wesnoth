package org.wesnoth.wizards.newfile;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import org.wesnoth.Messages;

/**
 * The "New" wizard page allows setting the container for the new file as well
 * as the file name. The page will only accept file name without the extension
 * OR with the extension that matches the expected one (cfg).
 */

public class NewConfigFilePage0 extends WizardPage
{
    private Text       containerText;

    private Text       fileText;

    private ISelection selection;

    /**
     * Constructor for SampleNewWizardPage.
     * 
     * @param selection
     *        The current selection
     */
    public NewConfigFilePage0( ISelection selection )
    {
        super( "wizardPage" ); //$NON-NLS-1$
        setTitle( Messages.NewConfigFilePage0_1 );
        setDescription( Messages.NewConfigFilePage0_2 );
        this.selection = selection;
    }

    /**
     * @see org.eclipse.jface.dialogs.IDialogPage#createControl(Composite)
     */
    @Override
    public void createControl( Composite parent )
    {
        Composite container = new Composite( parent, SWT.NULL );
        GridLayout layout = new GridLayout( );
        container.setLayout( layout );
        layout.numColumns = 3;
        layout.verticalSpacing = 9;
        Label label = new Label( container, SWT.NULL );
        label.setText( Messages.NewConfigFilePage0_3 );

        containerText = new Text( container, SWT.BORDER | SWT.SINGLE );
        GridData gd = new GridData( GridData.FILL_HORIZONTAL );
        containerText.setLayoutData( gd );
        containerText.addModifyListener( new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                dialogChanged( );
            }
        } );

        Button button = new Button( container, SWT.PUSH );
        button.setText( Messages.NewConfigFilePage0_4 );
        button.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                handleBrowse( );
            }
        } );
        label = new Label( container, SWT.NULL );
        label.setText( Messages.NewConfigFilePage0_5 );

        fileText = new Text( container, SWT.BORDER | SWT.SINGLE );
        gd = new GridData( GridData.FILL_HORIZONTAL );
        fileText.setLayoutData( gd );
        fileText.addModifyListener( new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                dialogChanged( );
            }
        } );
        initialize( );
        dialogChanged( );
        setControl( container );
    }

    /**
     * Tests if the current workbench selection is a suitable container to use.
     */

    private void initialize( )
    {
        if( selection != null && selection.isEmpty( ) == false
            && selection instanceof IStructuredSelection ) {
            IStructuredSelection ssel = ( IStructuredSelection ) selection;
            if( ssel.size( ) > 1 ) {
                return;
            }
            Object obj = ssel.getFirstElement( );
            if( obj instanceof IResource ) {
                IContainer container;
                if( obj instanceof IContainer ) {
                    container = ( IContainer ) obj;
                }
                else {
                    container = ( ( IResource ) obj ).getParent( );
                }
                containerText.setText( container.getFullPath( ).toString( ) );
            }
        }
        fileText.setText( "new_file.cfg" ); //$NON-NLS-1$
    }

    /**
     * Uses the standard container selection dialog to choose the new value for
     * the container field.
     */

    private void handleBrowse( )
    {
        ContainerSelectionDialog dialog = new ContainerSelectionDialog(
            getShell( ), ResourcesPlugin.getWorkspace( ).getRoot( ), false,
            Messages.NewConfigFilePage0_7 );
        if( dialog.open( ) == Window.OK ) {
            Object[] result = dialog.getResult( );
            if( result.length == 1 ) {
                containerText.setText( ( ( Path ) result[0] ).toString( ) );
            }
        }
    }

    /**
     * Ensures that both text fields are set.
     */

    private void dialogChanged( )
    {
        IResource container = ResourcesPlugin.getWorkspace( ).getRoot( )
            .findMember( new Path( getContainerName( ) ) );
        String fileName = getFileName( );

        if( getContainerName( ).length( ) == 0 ) {
            updateStatus( Messages.NewConfigFilePage0_8 );
            return;
        }
        if( container == null
            || ( container.getType( ) & ( IResource.PROJECT | IResource.FOLDER ) ) == 0 ) {
            updateStatus( Messages.NewConfigFilePage0_9 );
            return;
        }
        if( ! container.isAccessible( ) ) {
            updateStatus( Messages.NewConfigFilePage0_10 );
            return;
        }
        if( fileName.length( ) == 0 ) {
            updateStatus( Messages.NewConfigFilePage0_11 );
            return;
        }
        if( fileName.replace( '\\', '/' ).indexOf( '/', 1 ) > 0 ) {
            updateStatus( Messages.NewConfigFilePage0_12 );
            return;
        }
        int dotLoc = fileName.lastIndexOf( '.' );
        if( dotLoc != - 1 ) {
            String ext = fileName.substring( dotLoc + 1 );
            if( ext.equalsIgnoreCase( "cfg" ) == false ) { //$NON-NLS-1$
                updateStatus( Messages.NewConfigFilePage0_14 );
                return;
            }
        }
        updateStatus( null );
    }

    private void updateStatus( String message )
    {
        setErrorMessage( message );
        setPageComplete( message == null );
    }

    /**
     * @return The name of the container where the file is created
     */
    public String getContainerName( )
    {
        return containerText.getText( );
    }

    /**
     * @return The name of the new file
     */
    public String getFileName( )
    {
        return fileText.getText( );
    }
}

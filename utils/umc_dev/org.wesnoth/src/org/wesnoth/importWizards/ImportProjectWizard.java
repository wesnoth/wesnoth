package org.wesnoth.importWizards;

import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.ui.IImportWizard;

import org.wesnoth.Logger;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.wizards.WizardTemplate;

/**
 * Wizard to import wesnoth projects into workspace
 */
public class ImportProjectWizard extends WizardTemplate implements
    IImportWizard
{
    private ImportProjectPage page0_;

    /**
     * Creates a new {@link ImportProjectWizard}
     */
    public ImportProjectWizard( )
    {
        super( );
    }

    @Override
    public void addPages( )
    {
        super.addPages( );

        page0_ = new ImportProjectPage( );
        addPage( page0_ );
    }

    @Override
    public boolean performFinish( )
    {
        IRunnableWithProgress op = new IRunnableWithProgress( ) {
            @Override
            public void run( IProgressMonitor monitor )
            {
                ProjectUtils.createWesnothProject( page0_.getProjectName( ),
                    page0_.getProjectPath( ),
                    page0_.getSelectedInstallName( ), monitor );
                monitor.done( );
            }
        };
        try {
            getContainer( ).run( false, false, op );
        } catch( InterruptedException e ) {
            return false;
        } catch( InvocationTargetException e ) {
            Logger.getInstance( ).logException( e );
        }
        return true;
    }
}

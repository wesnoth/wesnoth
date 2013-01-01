/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.IWizardPage;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import org.wesnoth.WesnothPlugin;

/**
 * An abstract wizard template with some default functionality
 */
public abstract class WizardTemplate extends Wizard implements INewWizard
{
    protected IStructuredSelection selection_;
    protected IContainer           selectionContainer_;
    protected IWizardPage          lastPage_;
    protected boolean              isFinished_ = false;
    protected Object               data_       = null;
    protected String               objectName_ = "";   //$NON-NLS-1$

    /**
     * Creates a new {@link Wizard}
     */
    public WizardTemplate( )
    {
        setNeedsProgressMonitor( true );
        setHelpAvailable( true );
    }

    @Override
    public void init( IWorkbench workbench, IStructuredSelection selection )
    {
        this.selection_ = selection;
        initialize( );
    }

    @Override
    public void createPageControls( Composite pageContainer )
    {
        WesnothPlugin.getDefault( ).getWorkbench( ).getHelpSystem( )
            .setHelp( pageContainer, "org.wesnoth.wizardHelp" ); //$NON-NLS-1$

        super.createPageControls( pageContainer );
    }

    /**
     * Tests if the current workbench selection is a suitable campaign to use.
     */
    public void initialize( )
    {
        if( selection_ != null && selection_.isEmpty( ) == false ) {
            IStructuredSelection ssel = selection_;
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
                selectionContainer_ = container;
            }
        }
    }

    /**
     * Returns true if the wizard has finished.
     * 
     * @return True if the wizard has finished, false otherwise
     */
    public boolean isFinished( )
    {
        return isFinished_;
    }

    @Override
    public boolean canFinish( )
    {
        IWizardPage page = getContainer( ).getCurrentPage( );
        return super.canFinish( ) && page == lastPage_ && page.isPageComplete( );
    }

    @Override
    public void addPage( IWizardPage page )
    {
        super.addPage( page );

        lastPage_ = page;
    }

    /**
     * Gets the data associated with this wizard
     * 
     * @return The data associated with this wizard
     */
    public Object getData( )
    {
        return data_;
    }

    /**
     * Sets the data associated with this wizard
     * 
     * @param data
     *        The new data
     */
    public void setData( Object data )
    {
        data_ = data;
    }

    /**
     * Gets the name of the object created by the wizard
     * 
     * @return The object name string representation
     */
    public String getObjectName( )
    {
        return objectName_;
    }

    /**
     * Gets the container of the selection
     * 
     * @return An {@link IContainer} instance
     */
    public IContainer getSelectionContainer( )
    {
        return selectionContainer_;
    }

    /**
     * Sets the container of the selection
     * 
     * @param container
     *        The new selection container
     */
    public void setSelectionContainer( IContainer container )
    {
        this.selectionContainer_ = container;
    }
}

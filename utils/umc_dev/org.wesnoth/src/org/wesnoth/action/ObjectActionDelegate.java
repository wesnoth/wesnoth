/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.action;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

/**
 * An abstract generic ObjectActionDelegate implementation that
 * tracks the current selection
 */
public abstract class ObjectActionDelegate implements IObjectActionDelegate
{
    protected ISelection           selection_;
    protected IStructuredSelection structuredSelection_;
    protected IAction              action_;

    @Override
    public void selectionChanged( IAction action, ISelection selection )
    {
        selection_ = selection;
        action_ = action;
        if( selection instanceof IStructuredSelection ) {
            structuredSelection_ = ( IStructuredSelection ) selection;
        }
    }

    @Override
    public void setActivePart( IAction action, IWorkbenchPart targetPart )
    {
    }
}

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.action;

import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.dialogs.ProgressMonitorDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.WMLTools;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class UploadAddon implements IObjectActionDelegate
{
	public UploadAddon() {
	}

	@Override
	public void run(IAction action)
	{
		final String fullPath = WorkspaceUtils.getSelectedResource().getLocation().toOSString();
		ProgressMonitorDialog dialog = new ProgressMonitorDialog(Activator.getShell());
		try
		{
			dialog.run(false, false, new IRunnableWithProgress() {
				@Override
				public void run(IProgressMonitor monitor)
						throws InvocationTargetException, InterruptedException
				{
					monitor.beginTask("Uploading addon...", 50);
					monitor.worked(10);
					WMLTools.runWesnothAddonManager(fullPath, true, false);
					monitor.worked(40);
					monitor.done();
				}
			});
		} catch (InvocationTargetException e)
		{
			Logger.getInstance().logException(e);
		} catch (InterruptedException e)
		{
		}
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart)
	{
	}
}

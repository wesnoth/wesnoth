/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.action;

import java.io.OutputStream;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.dialogs.ProgressMonitorDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WMLTools;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class UploadAddon extends ObjectActionDelegate
{
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
					OutputStream consoleStream = GUIUtils.
						createConsole("Upload addon result: ", null, true).newOutputStream();
					WMLTools.runWesnothAddonManager(fullPath,
							new OutputStream[] { consoleStream }, new OutputStream[] { consoleStream });
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
}

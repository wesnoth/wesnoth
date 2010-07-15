/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.resources.IFolder;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.globalactions.MapActions;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class ImportMapHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event)
	{
		MapActions.importMap();
		return null;
	}

	@Override
	public boolean isHandled()
	{
		IFolder selectedFolder = WorkspaceUtils.getSelectedFolder(WorkspaceUtils.getWorkbenchWindow());
		if (selectedFolder == null)
		{
			Logger.getInstance().log("no directory selected (importMapHandler)",
					"Please select a folder before proceeding.");
		}

		if (!selectedFolder.getName().equals("maps"))
		{
			MessageBox confirmBox = new MessageBox(Activator.getShell(),
					SWT.ICON_QUESTION | SWT.YES | SWT.NO);
			confirmBox.setMessage("A map should be imported into a 'maps' folder. Do you want to proceed?");

			if (confirmBox.open() == SWT.NO)
				return false;
		}

		return (selectedFolder.getName().equals("maps"));
	}
}

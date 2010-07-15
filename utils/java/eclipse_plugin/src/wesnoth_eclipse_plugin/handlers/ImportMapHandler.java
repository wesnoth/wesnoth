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
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.resources.IFolder;

import wesnoth_eclipse_plugin.globalactions.MapActions;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class ImportMapHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		MapActions.importMap();
		return null;
	}

	@Override
	public boolean isHandled()
	{
		IFolder selectedFolder = WorkspaceUtils.getSelectedFolder(WorkspaceUtils.getWorkbenchWindow());

		if (!selectedFolder.getName().equals("maps"))
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
					"You need to select a \"maps\" folder before importing anything");
		}

		return (selectedFolder.getName().equals("maps"));
	}
}

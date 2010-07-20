/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.io.File;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Logger;

public class MapUtils
{
	/**
	 * Import a map file into the currently selected directory
	 */
	public static void importMap()
	{
		if (WorkspaceUtils.getSelectedFolder() == null)
		{
			Logger.getInstance().log("no directory selected (importMap)",
				"Please select a directory before importing a map");
			return;
		}

		FileDialog mapDialog = new FileDialog(Activator.getShell(), SWT.OPEN);
		mapDialog.setText("Import wesnoth map");
		mapDialog.setFilterExtensions(new String[] {"*.map" });
		String file = mapDialog.open();

		if (file == null)
			return;

		try
		{
			File source = new File(file);
			File target = new File(WorkspaceUtils.getSelectedFolder().getLocation().toOSString() +
					Path.SEPARATOR + source.getName());

			if (target.exists())
			{
				if (GUIUtils.showMessageBox("There is already an existing map with the same name. Overwrite?",
						SWT.ICON_QUESTION | SWT.YES | SWT.NO) == SWT.NO)
					return;
			}

			ResourceUtils.copyTo(source, target);
			WorkspaceUtils.getSelectedFolder().refreshLocal(IResource.DEPTH_INFINITE, null);
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}
}

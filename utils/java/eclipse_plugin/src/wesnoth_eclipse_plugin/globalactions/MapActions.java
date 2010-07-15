/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.globalactions;

import java.io.File;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.MessageBox;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.ResourceUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class MapActions
{
	/**
	 * Import a map file into the current directory
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
				MessageBox confirmBox = new MessageBox(Activator.getShell(),
						SWT.ICON_QUESTION | SWT.YES | SWT.NO);
				confirmBox.setMessage("There is already an existing map with the same name. Overwrite?");

				if (confirmBox.open() == SWT.NO)
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

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.File;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.wesnoth.Activator;
import org.wesnoth.Logger;
import org.wesnoth.Messages;


public class MapUtils
{
	/**
	 * Import a map file into the currently selected directory
	 */
	public static void importMap()
	{
		if (WorkspaceUtils.getSelectedFolder() == null)
		{
			Logger.getInstance().log(Messages.MapUtils_0,
				Messages.MapUtils_1);
			return;
		}

		FileDialog mapDialog = new FileDialog(Activator.getShell(), SWT.OPEN);
		mapDialog.setText(Messages.MapUtils_2);
		mapDialog.setFilterExtensions(new String[] {"*.map" }); //$NON-NLS-1$
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
				if (GUIUtils.showMessageBox(Messages.MapUtils_4,
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

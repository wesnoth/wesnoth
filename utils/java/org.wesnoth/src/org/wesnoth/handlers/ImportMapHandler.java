/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.resources.IFolder;
import org.eclipse.swt.SWT;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.MapUtils;
import org.wesnoth.utils.WorkspaceUtils;


public class ImportMapHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event)
	{
		IFolder selectedFolder = WorkspaceUtils.getSelectedFolder();
		if (selectedFolder == null)
		{
			Logger.getInstance().log(Messages.ImportMapHandler_0,
				Messages.ImportMapHandler_1);
		}

		if (!selectedFolder.getName().equals("maps") && //$NON-NLS-1$
				GUIUtils.showMessageBox(Messages.ImportMapHandler_3 +
						Messages.ImportMapHandler_4,
						SWT.ICON_QUESTION | SWT.YES | SWT.NO) == SWT.NO)
			return null;

		MapUtils.importMap();
		return null;
	}
}

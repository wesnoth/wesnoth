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
import org.eclipse.core.resources.IFile;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.utils.GameUtils;
import org.wesnoth.utils.WorkspaceUtils;


public class OpenMapInEditorHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event)
	{
		IFile selectedFile = WorkspaceUtils.getSelectedFile();
		if (selectedFile == null)
		{
			Logger.getInstance().log(Messages.OpenMapInEditorHandler_0,
					Messages.OpenMapInEditorHandler_1);
			return null;
		}

		if (!selectedFile.getName().endsWith(".map")) //$NON-NLS-1$
		{
			Logger.getInstance().log(Messages.OpenMapInEditorHandler_3+selectedFile.getName(),
					Messages.OpenMapInEditorHandler_4);
			return null;
		}

		GameUtils.startEditor(selectedFile.getLocation().toOSString());
		return null;
	}
}

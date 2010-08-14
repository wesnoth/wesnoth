/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.action;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.action.IAction;
import org.wesnoth.utils.PreprocessorUtils;
import org.wesnoth.utils.WMLTools;
import org.wesnoth.utils.WorkspaceUtils;
import org.wesnoth.utils.WMLTools.Tools;


public class RunWMLLintOnPreprocFile extends ObjectActionDelegate
{
	@Override
	public void run(IAction action)
	{
		IFile file = WorkspaceUtils.getSelectedFile(WorkspaceUtils.getWorkbenchWindow());
		PreprocessorUtils.getInstance().preprocessFile(file, null);

		WMLTools.runWMLToolAsWorkspaceJob(Tools.WMLLINT,
				PreprocessorUtils.getInstance().getPreprocessedFilePath(file,	false, false).toString());
	}
}

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.action;

import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.utils.WMLTools;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class RunWMLScopeOnSelection implements IObjectActionDelegate
{
	public RunWMLScopeOnSelection() {
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart)
	{
	}

	@Override
	public void run(IAction action)
	{

		IEditorReference[] files =
				WorkspaceUtils.getWorkbenchWindow().getPages()[0].getEditorReferences();

		for (IEditorReference file : files)
		{
			if (file.isDirty())
				file.getEditor(false).doSave(null);
		}

		final String path = WorkspaceUtils.getSelectedResource().getLocation().toOSString();
		WorkspaceJob job = new WorkspaceJob("Running WMLScope") {
			private ExternalToolInvoker	tool	= WMLTools.runWMLScope(path, true, true);

			@Override
			protected void canceling()
			{
				tool.kill();
			}

			@Override
			public IStatus runInWorkspace(final IProgressMonitor monitor)
			{
				monitor.beginTask("wmlscope is running...", 50);
				monitor.worked(10);
				tool.waitForTool();
				monitor.done();
				return Status.OK_STATUS;
			}
		};
		job.schedule();
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
	}
}

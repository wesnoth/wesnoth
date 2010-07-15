/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.action;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.utils.EditorUtils;
import wesnoth_eclipse_plugin.utils.WMLTools;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class RunWMLIndentOnSelection implements IObjectActionDelegate
{

	public RunWMLIndentOnSelection() {
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart)
	{
	}

	@Override
	public void run(IAction action)
	{
		WorkspaceJob job = new WorkspaceJob("Running WMLIndent") {
			@Override
			public IStatus runInWorkspace(final IProgressMonitor monitor)
			{
				final IEditorReference[] files =
						WorkspaceUtils.getWorkbenchWindow().getPages()[0].getEditorReferences();

				monitor.beginTask("wmlindent", files.length * 5 + 50);
				Display.getDefault().asyncExec(new Runnable() {
					@Override
					public void run()
					{
						monitor.subTask("saving files...");
						for (IEditorReference file : files)
						{
							monitor.worked(5);
							if (file.isDirty())
								file.getEditor(false).doSave(null);
						}

						IFile selFile = WorkspaceUtils.getSelectedFile();
						monitor.subTask("wmlindent");
						if (selFile != null)
						{
							EditorUtils.openEditor(selFile, true);
							String stdin = EditorUtils.getEditorDocument().get();
							EditorUtils.replaceEditorText(WMLTools.runWMLIndent(null, stdin, false, false, false));
						}
						else
						// project selection
						{
							// run wmlindent on project
							IProject project = WorkspaceUtils.getSelectedProject();
							WMLTools.runWMLIndent(project.getLocation().toOSString(), null, false, true, false);
						}
						monitor.worked(50);
					};
				});
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

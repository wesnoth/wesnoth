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
import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;

import wesnoth_eclipse_plugin.Logger;

public class TestHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		//String stdin = EditorUtils.getEditorDocument().get();
		//EditorUtils.replaceEditorText(WMLTools.runWMLIndent(null, stdin, false, false, false));
		//		IEditorReference[] files =
		//				Activator.getDefault().getWorkbench().getActiveWorkbenchWindow().getPages()[0].getEditorReferences();
		//		for (IEditorReference file : files)
		//		{
		//			if (file.isDirty())
		//				file.getEditor(false).doSave(null);
		//		}
		//		ProgressMonitorDialog dialog = new ProgressMonitorDialog(Activator.getShell());
		//		try
		//		{
		//			dialog.run(true, true, new IRunnableWithProgress() {
		//				@Override
		//				public void run(IProgressMonitor monitor)
		//				{
		//					monitor.beginTask("Some nice progress message here ...", 100);
		//					// execute the task ...
		//					try
		//					{
		//						Thread.sleep(2000);
		//					} catch (InterruptedException e)
		//					{
		//						Logger.getInstance().logException(e);
		//					}
		//					monitor.done();
		//				}
		//			});
		//		} catch (InvocationTargetException e)
		//		{
		//			Logger.getInstance().logException(e);
		//		} catch (InterruptedException e)
		//		{
		//			Logger.getInstance().logException(e);
		//		}
		//		UIJob job1 = new UIJob("My the job") {
		//
		//			@Override
		//			public IStatus runInUIThread(IProgressMonitor monitor)
		//			{
		//				monitor.beginTask("Some nice progress message here ...", 100);
		//				// execute the task ...try
		//				try
		//				{
		//					Thread.sleep(2000);
		//				} catch (InterruptedException e)
		//				{
		//					Logger.getInstance().logException(e);
		//				}
		//				monitor.done();
		//				return Status.OK_STATUS;
		//			}
		//		};
		//		//job1.schedule();
		//		WorkbenchJob job2 = new WorkbenchJob("asdasdd ") {
		//
		//			@Override
		//			public IStatus runInUIThread(IProgressMonitor monitor)
		//			{
		//				monitor.beginTask("Some nice progress message here ...", 100);
		//				// execute the task ...try
		//				try
		//				{
		//					Thread.sleep(2000);
		//				} catch (InterruptedException e)
		//				{
		//					Logger.getInstance().logException(e);
		//				}
		//				monitor.done();
		//				return Status.OK_STATUS;
		//			}
		//		};
		//		job2.schedule();
		new WorkspaceJob("My new job") {
			@Override
			public IStatus runInWorkspace(IProgressMonitor monitor) throws CoreException
			{
				monitor.beginTask("Some nice progress message here ...", 100);
				// execute the task ...try
				try
				{
					Thread.sleep(2000);
				} catch (InterruptedException e)
				{
					Logger.getInstance().logException(e);
				}
				monitor.done();
				return Status.OK_STATUS;
			}
		}.schedule();
		//job.schedule();
		return null;
	}
}

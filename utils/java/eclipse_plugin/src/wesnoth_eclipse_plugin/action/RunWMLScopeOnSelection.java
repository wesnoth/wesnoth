/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.action;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.progress.WorkbenchJob;

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

		WorkbenchJob job = new WorkbenchJob("Running WMLScope") {
			@Override
			public IStatus runInUIThread(IProgressMonitor monitor)
			{
				final IEditorReference[] files =
						WorkspaceUtils.getWorkbenchWindow().getPages()[0].getEditorReferences();

				monitor.beginTask("wmlscope", files.length * 5 + 50);
				monitor.subTask("saving files...");
				for (IEditorReference file : files)
				{
					monitor.worked(5);
					if (file.isDirty())
						file.getEditor(false).doSave(null);
				}

				IFile selFile = WorkspaceUtils.getSelectedFile();
				monitor.subTask("wmlscope");
				if (selFile != null)
				{
					WMLTools.runWMLScope(selFile.getLocation().toOSString(), false, true);
				}
				else
				// project selection
				{
					// run wmlscope on project
					IProject project = WorkspaceUtils.getSelectedProject();
					WMLTools.runWMLScope(project.getLocation().toOSString(), true, true);

				}
				monitor.worked(50);
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

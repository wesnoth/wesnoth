/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.action;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
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
		IFile selFile = WorkspaceUtils.getSelectedFile();
		if (selFile != null)
		{
			EditorUtils.openEditor(selFile, true);
			String stdin = EditorUtils.getEditorDocument().get();
			EditorUtils.replaceEditorText(WMLTools.runWMLIndent(null, stdin, false, false, false));
			//WMLTools.runWMLIndent(WorkspaceUtils.getPathRelativeToUserDir(WorkspaceUtils.getSelectedFile()), null,
			//		false, true, false);
		}
		else
		// project selection
		{
			// save currently opened files
			final IEditorReference[] files =
					WorkspaceUtils.getWorkbenchWindow().getPages()[0].getEditorReferences();
			for (IEditorReference file : files)
			{
				if (file.isDirty())
					file.getEditor(false).doSave(null);
			}

			// run wmlindent on project
			IProject project = WorkspaceUtils.getSelectedProject();
			WMLTools.runWMLIndent(project.getLocation().toOSString(), null, false, true, false);
		}
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
	}
}

/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.IWorkbenchWindow;

import wesnoth_eclipse_plugin.Activator;

public class WorkspaceUtils
{
	public static IProject getSelectedProject(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IProject))
			return null;

		return (IProject)selection.getFirstElement();
	}

	public static IFolder getSelectedFolder(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IFolder))
			return null;

		return (IFolder)selection.getFirstElement();
	}

	public static IFile getSelectedFile(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IFile))
			return null;

		return (IFile)selection.getFirstElement();
	}

	public static IStructuredSelection getSelectedStructuredSelection(IWorkbenchWindow window)
	{
		if (window == null)
		{
			System.out.println("WokbenchWindow NULL!!");
			return null;
		}

		if (!(window.getSelectionService().getSelection() instanceof IStructuredSelection))
			return null;
		return (IStructuredSelection)window.getSelectionService().getSelection();
	}
	public static IWorkbenchWindow getWorkbenchWindow()
	{
		if (Activator.getDefault().getWorkbench().getWorkbenchWindowCount() == 0)
			return null;
		return Activator.getDefault().getWorkbench().getWorkbenchWindows()[0];
	}
}

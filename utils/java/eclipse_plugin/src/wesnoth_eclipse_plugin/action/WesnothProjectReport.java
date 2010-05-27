package wesnoth_eclipse_plugin.action;

import java.io.File;

import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.SWT;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class WesnothProjectReport implements IObjectActionDelegate
{
	public WesnothProjectReport(){ }

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart){
	}

	@Override
	public void run(IAction action)
	{
		IProject project = WorkspaceUtils.getSelectedProject(WorkspaceUtils.getWorkbenchWindow());
		IFolder scenariosFolder = project.getFolder("scenarios");
		IFolder mapsFolder = project.getFolder("maps");
		IFolder unitsFolder = project.getFolder("units");

		String simpleReport = String.format("Scenarios: %d \nMaps: %d \nUnits: %d",
				 new File(scenariosFolder.getLocation().toOSString()).listFiles().length,
				 new File(mapsFolder.getLocation().toOSString()).listFiles().length,
				 new File(unitsFolder.getLocation().toOSString()).listFiles().length);

		GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), simpleReport,SWT.ICON_INFORMATION);
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
	}
}

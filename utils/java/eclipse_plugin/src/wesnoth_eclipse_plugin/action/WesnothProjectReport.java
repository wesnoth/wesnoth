/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
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
		IProject project = WorkspaceUtils.getSelectedProject();
		if (project == null)
		{
			GUIUtils.showMessageBox("Please select a project first.");
			return;
		}

		int[] statistics = new int[3];
		IFolder scenariosFolder = project.getFolder("scenarios");
		if (scenariosFolder.exists())
			statistics[0] = new File(scenariosFolder.getLocation().toOSString()).listFiles().length;

		IFolder mapsFolder = project.getFolder("maps");
		if (mapsFolder.exists())
			statistics[1] = new File(mapsFolder.getLocation().toOSString()).listFiles().length;

		IFolder unitsFolder = project.getFolder("units");
		if (unitsFolder.exists())
			statistics[2] = new File(unitsFolder.getLocation().toOSString()).listFiles().length;

		String simpleReport = String.format("Scenarios: %d \nMaps: %d \nUnits: %d",
				 statistics[0],statistics[1], statistics[2]);

		GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), simpleReport,
				SWT.ICON_INFORMATION);
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
	}
}

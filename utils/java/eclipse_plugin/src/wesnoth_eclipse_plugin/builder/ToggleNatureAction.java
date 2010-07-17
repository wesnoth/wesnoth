/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.builder;

import java.util.Iterator;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.xtext.ui.XtextProjectHelper;

import wesnoth_eclipse_plugin.Logger;

public class ToggleNatureAction implements IObjectActionDelegate
{
	private ISelection	selection_;

	@Override
	@SuppressWarnings("rawtypes")
	public void run(IAction action)
	{
		if (selection_ instanceof IStructuredSelection)
		{
			for (Iterator it = ((IStructuredSelection) selection_).iterator(); it.hasNext();)
			{
				Object element = it.next();
				IProject project = null;
				if (element instanceof IProject)
				{
					project = (IProject) element;
				}
				else if (element instanceof IAdaptable)
				{
					project = (IProject) ((IAdaptable) element).getAdapter(IProject.class);
				}
				if (project != null)
				{
					toggleNature(project);
				}
			}
		}
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
		this.selection_ = selection;
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart)
	{
	}

	/**
	 * Toggles sample nature on a project
	 *
	 * @param project
	 *        to have sample nature added or removed
	 */
	public void toggleNature(IProject project)
	{
		try
		{
			IProjectDescription description = project.getDescription();
			String[] natures = description.getNatureIds();

			boolean removed = false;
			for (int i = 0; i < natures.length; ++i)
			{
				if (WesnothProjectNature.WESNOTH_NATURE_ID.equals(natures[i]))
				{
					// Remove the nature
					String[] newNatures = new String[natures.length - 1];
					System.arraycopy(natures, 0, newNatures, 0, i);
					System.arraycopy(natures, i + 1, newNatures, i, natures.length - i - 1);
					description.setNatureIds(newNatures);
					project.setDescription(description, null);
					removed = true;
				}
				if (XtextProjectHelper.NATURE_ID.equals(natures[i]))
				{
					// Remove the nature
					String[] newNatures = new String[natures.length - 1];
					System.arraycopy(natures, 0, newNatures, 0, i);
					System.arraycopy(natures, i + 1, newNatures, i, natures.length - i - 1);
					description.setNatureIds(newNatures);
					project.setDescription(description, null);
					removed = true;
				}
			}

			if (removed == true)
				return;

			// Add the natures
			String[] newNatures = new String[natures.length + 2];
			System.arraycopy(natures, 0, newNatures, 0, natures.length);
			newNatures[natures.length] = WesnothProjectNature.WESNOTH_NATURE_ID;
			newNatures[natures.length + 1] = XtextProjectHelper.NATURE_ID;
			description.setNatureIds(newNatures);
			project.setDescription(description, null);
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}

}

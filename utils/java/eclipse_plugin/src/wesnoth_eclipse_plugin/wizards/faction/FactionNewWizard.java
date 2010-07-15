/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.faction;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class FactionNewWizard extends NewWizardTemplate
{
	FactionPage0	page0_;
	FactionPage1	page1_;

	public FactionNewWizard() {
		setWindowTitle("Create a new faction");
		setNeedsProgressMonitor(true);
	}

	@Override
	public void addPages()
	{
		page0_ = new FactionPage0(selection_);
		addPage(page0_);

		page1_ = new FactionPage1();
		addPage(page1_);

		super.addPages();
	}

	@Override
	public boolean canFinish()
	{
		return getContainer().getCurrentPage().isPageComplete();
	}

	@Override
	public boolean performFinish()
	{
		final String containerName = page0_.getDirectoryName();
		final String fileName = page0_.getFileName();
		IRunnableWithProgress op = new IRunnableWithProgress() {
			@Override
			public void run(IProgressMonitor monitor) throws InvocationTargetException
			{
				try
				{
					doFinish(containerName, fileName, monitor);
				} catch (CoreException e)
				{
					throw new InvocationTargetException(e);
				} finally
				{
					monitor.done();
				}
			}
		};
		try
		{
			getContainer().run(false, false, op);
		} catch (InterruptedException e)
		{
			return false;
		} catch (InvocationTargetException e)
		{
			Throwable realException = e.getTargetException();
			MessageDialog.openError(getShell(), "Error", realException.getMessage());
			return false;
		}
		return true;
	}

	private void doFinish(String containerName, String fileName, IProgressMonitor monitor) throws CoreException
	{
		// create a sample file
		monitor.beginTask("Creating " + fileName, 10);
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IResource resource = root.findMember(new Path(containerName));

		IContainer container = (IContainer) resource;
		final IFile file = container.getFile(new Path(fileName));

		try
		{
			InputStream stream = getFactionStream();

			if (stream == null)
				return;

			if (file.exists())
			{
				file.setContents(stream, true, true, monitor);
			}
			else
			{
				file.create(stream, true, monitor);
			}

			stream.close();
		} catch (IOException e)
		{
			e.printStackTrace();
		}

		monitor.worked(5);
		monitor.setTaskName("Opening file for editing...");
		getShell().getDisplay().asyncExec(new Runnable() {
			@Override
			public void run()
			{
				IWorkbenchPage page = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
				try
				{
					IDE.openEditor(page, file, true);
				} catch (PartInitException e)
				{
				}
			}
		});
		monitor.worked(5);
	}

	private InputStream getFactionStream()
	{
		ArrayList<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();

		params.add(new ReplaceableParameter("$$faction_id", page0_.getFactionId()));
		params.add(new ReplaceableParameter("$$faction_name", page0_.getFactionName()));
		params.add(new ReplaceableParameter("$$faction_type", page0_.getType()));
		params.add(new ReplaceableParameter("$$leader", page0_.getLeader()));
		params.add(new ReplaceableParameter("$$random_leader", page0_.getRandomLeader()));
		params.add(new ReplaceableParameter("$$terrain_liked", page0_.getTerrainLiked()));

		params.add(new ReplaceableParameter("$$random_faction", String.valueOf(page1_.getIsRandomFaction())));
		params.add(new ReplaceableParameter("$$choices", page1_.getChoices()));
		params.add(new ReplaceableParameter("$$except", page1_.getExcept()));

		String template = TemplateProvider.getInstance().getProcessedTemplate("faction", params);

		if (template == null)
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Template for \"faction\" not found.");
			return null;
		}

		return new ByteArrayInputStream(template.getBytes());
	}
}

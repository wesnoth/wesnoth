/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.era;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class EraNewWizard extends NewWizardTemplate
{
	EraPage0	page0_;

	public EraNewWizard() {
		setWindowTitle("Create a new era");
		setNeedsProgressMonitor(true);
	}

	@Override
	public void addPages()
	{
		page0_ = new EraPage0();
		addPage(page0_);

		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		final String containerName = page0_.getDirectoryName();
		final String fileName = page0_.getFileName();
		IRunnableWithProgress op = new IRunnableWithProgress() {
			@Override
			public void run(IProgressMonitor monitor)
			{
				try
				{
					doFinish(containerName, fileName, monitor);
				} catch (CoreException e)
				{
					Logger.getInstance().logException(e);
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
			Logger.getInstance().logException(e);
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
			InputStream stream = getEraStream();

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
			Logger.getInstance().logException(e);
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

		// refresh the addons project
		IProject uaproj = WorkspaceUtils.getUserAddonsProject();
		if (uaproj != null)
		{
			uaproj.refreshLocal(IResource.DEPTH_ONE, new NullProgressMonitor());
		}

		monitor.done();
	}

	private InputStream getEraStream()
	{
		ArrayList<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();

		params.add(new ReplaceableParameter("$$era_id", page0_.getEraID()));
		params.add(new ReplaceableParameter("$$era_name", page0_.getEraName()));
		params.add(new ReplaceableParameter("$$require_era", String.valueOf(page0_.getRequiresEra())));

		String template = TemplateProvider.getInstance().getProcessedTemplate("era", params);

		if (template == null)
		{
			Logger.getInstance().log("'era' template not found",
					"Template for \"era\" not found.");
			return null;
		}

		return new ByteArrayInputStream(template.getBytes());
	}
}

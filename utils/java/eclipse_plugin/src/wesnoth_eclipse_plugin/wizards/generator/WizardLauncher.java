/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.generator;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.operation.IRunnableWithProgress;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.EditorUtils;
import wesnoth_eclipse_plugin.utils.WizardUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;

public class WizardLauncher extends NewWizardTemplate
{
	WizardLauncherPage0	page0_;
	WizardLauncherPage1	page1_;
	WizardGenerator		wizard_;

	public WizardLauncher() {
		setWindowTitle("Wizard launcher");
		setNeedsProgressMonitor(true);
	}

	@Override
	public void addPages()
	{
		page0_ = new WizardLauncherPage0();
		addPage(page0_);

		page1_ = new WizardLauncherPage1();
		addPage(page1_);

		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		wizard_ = new WizardGenerator(page1_.getTagDescription() +
							" new wizard", page1_.getTagName(), (byte) 0);
		WizardUtils.launchWizard(wizard_, getShell(), selection_);
		if (!wizard_.isFinished())
			return false;

		IRunnableWithProgress op = new IRunnableWithProgress() {
			@Override
			public void run(IProgressMonitor monitor) throws InvocationTargetException
			{
				doFinish(monitor);
				monitor.done();
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
			return false;
		}
		return true;
	}

	private void doFinish(IProgressMonitor monitor)
	{
		try
		{
			// The file is opened in the editor -> just copy-paste the text
			if (!(page0_.getIsTargetNewFile()))
			{
				EditorUtils.writeInEditor(EditorUtils.getEditedFile(), wizard_.getData().toString());
				return;
			}

			final String containerName = page0_.getDirectoryName();
			final String fileName = page0_.getFileName();

			// create the file
			monitor.beginTask("Creating " + fileName, 10);
			IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
			IResource resource = root.findMember(new Path(containerName));

			IContainer container = (IContainer) resource;
			final IFile file = container.getFile(new Path(fileName));

			InputStream stream = new ByteArrayInputStream(wizard_.getData().toString().getBytes());

			if (file.exists())
			{
				file.setContents(stream, true, true, monitor);
			}
			else
			{
				file.create(stream, true, monitor);
			}

			stream.close();

			monitor.worked(5);
			monitor.setTaskName("Opening file for editing...");
			getShell().getDisplay().asyncExec(new Runnable() {
				@Override
				public void run()
					{
						EditorUtils.openEditor(file, true);
					}
			});
			monitor.worked(5);
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}
}

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.builder;

import org.eclipse.core.resources.ICommand;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IProjectNature;
import org.eclipse.core.runtime.CoreException;

import wesnoth_eclipse_plugin.Constants;

public class WesnothProjectNature implements IProjectNature
{
	private IProject			project;

	@Override
	public void configure() throws CoreException
	{
		IProjectDescription desc = project.getDescription();
		ICommand[] commands = desc.getBuildSpec();

		boolean wesnothConfigured = false;
		boolean xtextConfigured = false;
		int configured = 0;
		for (int i = 0; i < commands.length; ++i)
		{
			if (commands[i].getBuilderName().equals(Constants.BUIILDER_WESNOTH))
			{
				wesnothConfigured = true;
				configured++;
			}
			if (commands[i].getBuilderName().equals(Constants.BUILDER_XTEXT))
			{
				xtextConfigured = true;
				configured++;
			}
		}
		if (configured == 2)
			return;

		ICommand[] newCommands = new ICommand[commands.length + (2 - configured)];
		System.arraycopy(commands, 0, newCommands, 0, commands.length);
		if (wesnothConfigured == false)
		{
			ICommand command = desc.newCommand();
			command.setBuilderName(Constants.BUIILDER_WESNOTH);
			newCommands[newCommands.length - 1] = command;
		}
		if (xtextConfigured == false)
		{
			ICommand command = desc.newCommand();
			command.setBuilderName(Constants.BUILDER_XTEXT);
			newCommands[newCommands.length - (2 - configured)] = command;
		}
		desc.setBuildSpec(newCommands);
		project.setDescription(desc, null);
	}

	@Override
	public void deconfigure() throws CoreException
	{
		IProjectDescription description = getProject().getDescription();
		ICommand[] commands = description.getBuildSpec();
		for (int i = 0; i < commands.length; ++i)
		{
			if (commands[i].getBuilderName().equals(Constants.BUIILDER_WESNOTH) ||
				commands[i].getBuilderName().equals(Constants.BUILDER_XTEXT))
			{
				ICommand[] newCommands = new ICommand[commands.length - 1];
				System.arraycopy(commands, 0, newCommands, 0, i);
				System.arraycopy(commands, i + 1, newCommands, i,
						commands.length - i - 1);
				description.setBuildSpec(newCommands);
				project.setDescription(description, null);
			}
		}
	}

	@Override
	public IProject getProject()
	{
		return project;
	}

	@Override
	public void setProject(IProject project)
	{
		this.project = project;
	}

}

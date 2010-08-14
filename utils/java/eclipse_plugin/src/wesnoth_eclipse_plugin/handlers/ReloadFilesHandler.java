/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.handlers;

import java.util.Map.Entry;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.resources.IProject;

import wesnoth_eclipse_plugin.schema.SchemaParser;
import wesnoth_eclipse_plugin.templates.TemplateProvider;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ProjectCache;
import wesnoth_eclipse_plugin.utils.ProjectUtils;

public class ReloadFilesHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event)
	{
		SchemaParser.getInstance().parseSchema(true);
		TemplateProvider.getInstance().loadTemplates();

		// reload the cache only for already loaded files
		for(Entry<IProject, ProjectCache> cache :
				ProjectUtils.getProjectCaches().entrySet())
		{
			cache.getValue().readDefines(true);
		}

		GUIUtils.showInfoMessageBox("Files reloaded.");
		return null;
	}
}

package wesnoth_eclipse_plugin.action;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.action.IAction;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.templates.ReplaceableParameter;
import wesnoth_eclipse_plugin.templates.TemplateProvider;
import wesnoth_eclipse_plugin.utils.ResourceUtils;

public class RegenerateBuildFiles extends ObjectActionDelegate
{
	public RegenerateBuildFiles() { }

	@SuppressWarnings("rawtypes")
	@Override
	public void run(IAction action)
	{
		if (structuredSelection_ == null)
			return;

		for (Iterator it = structuredSelection_.iterator(); it.hasNext();)
		{
			Object element = it.next();
			if (element instanceof IProject)
			{
				ArrayList<ReplaceableParameter> param = new ArrayList<ReplaceableParameter>();
				param.add(new ReplaceableParameter("$$project_name", (((IProject)element).getName())));
				param.add(new ReplaceableParameter("$$project_dir_name", ((IProject)element).getName()));
				ResourceUtils.createFile((IProject)element, "build.xml",
						TemplateProvider.getInstance().getProcessedTemplate("build_xml", param), true);
				try
				{
					((IProject)element).refreshLocal(IResource.DEPTH_ONE, new NullProgressMonitor());
				}
				catch (CoreException e)
				{
					Logger.getInstance().logException(e);
				}
			}
		}
	}
}

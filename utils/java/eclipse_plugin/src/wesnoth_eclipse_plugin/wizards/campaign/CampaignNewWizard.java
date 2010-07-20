/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.campaign;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.operation.IRunnableWithProgress;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.builder.WesnothProjectNature;
import wesnoth_eclipse_plugin.utils.Pair;
import wesnoth_eclipse_plugin.utils.ResourceUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class CampaignNewWizard extends NewWizardTemplate
{
	protected CampaignPage0 page0_;
	protected CampaignPage1 page1_;
	protected CampaignPage2 page2_;

	@Override
	public void addPages()
	{
		page0_ = new CampaignPage0();
		addPage(page0_);

		page1_ = new CampaignPage1();
		addPage(page1_);

		page2_ = new CampaignPage2();
		addPage(page2_);

		super.addPages();
	}

	public CampaignNewWizard() {
		setWindowTitle("Create a new Campaign");
		setNeedsProgressMonitor(true);
	}

	@Override
	public boolean performFinish()
	{
		try
		{
			// let's create the project
			getContainer().run(false, false, new IRunnableWithProgress() {
				@Override
				public void run(IProgressMonitor monitor) throws InvocationTargetException, InterruptedException
				{
					createProject(monitor);
					monitor.done();
				}
			});
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
			return false;
		}
		return true;
	}

	public void createProject(IProgressMonitor monitor)
	{
		monitor.beginTask("Creating the project structure...", 15);

		try
		{
			IProject currentProject = page0_.getProjectHandle();

			// the project
			currentProject.create(new NullProgressMonitor());
			currentProject.open(null);
			monitor.worked(2);

			// add the nature to the project
			IProjectDescription description = currentProject.getDescription();
			description.setNatureIds(new String[] { WesnothProjectNature.WESNOTH_NATURE_ID,
					WesnothProjectNature.XTEXT_NATURE_ID});
			currentProject.setDescription(description, null);

			String campaignStructure = prepareTemplate("campaign_structure");
			if (campaignStructure == null)
				return;

			List<Pair<String, String>> files;
			List<String> dirs;
			Pair<List<Pair<String, String>>, List<String>> tmp = TemplateProvider.getInstance().getFilesDirectories(campaignStructure);
			files = tmp.First;
			dirs = tmp.Second;

			for (Pair<String, String> file : files)
			{
				ResourceUtils.createFile(currentProject, file.First, prepareTemplate(file.Second), true);
				monitor.worked(1);
			}
			for (String dir : dirs)
			{
				ResourceUtils.createFolder(currentProject, dir);
				monitor.worked(1);
			}

			// create the '.wesnoth' file to store some campaign related info
			String wesnothFile = "";
			wesnothFile += "settings\n";
			wesnothFile += "difficulties=" + page2_.getDifficulties() + "\n";
			wesnothFile += "end_settings\n";

			ResourceUtils.createFile(currentProject, ".wesnoth", wesnothFile, true);
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}

		monitor.done();
	}

	private String prepareTemplate(String templateName)
	{
		ArrayList<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();

		params.add(new ReplaceableParameter("$$campaign_name", page1_.getCampaignName()));
		params.add(new ReplaceableParameter("$$author", page1_.getAuthor()));
		params.add(new ReplaceableParameter("$$version", page1_.getVersion()));
		params.add(new ReplaceableParameter("$$description", page1_.getCampaignDescription()));
		params.add(new ReplaceableParameter("$$icon", page1_.getIconPath()));
		params.add(new ReplaceableParameter("$$email", page1_.getEmail()));
		params.add(new ReplaceableParameter("$$passphrase", page1_.getPassphrase()));
		params.add(new ReplaceableParameter("$$translations_dir", page1_.getTranslationDir()));

		params.add(new ReplaceableParameter("$$campaign_id", page2_.getCampaignId()));
		params.add(new ReplaceableParameter("$$abrev", page2_.getAbbrev()));
		params.add(new ReplaceableParameter("$$define", page2_.getDefine()));
		params.add(new ReplaceableParameter("$$difficulties", page2_.getDifficulties()));
		params.add(new ReplaceableParameter("$$first_scenario", page2_.getFirstScenario()));

		params.add(new ReplaceableParameter("$$project_name", page0_.getProjectName()));
		params.add(new ReplaceableParameter("$$project_dir_name", page0_.getProjectName()));
		params.add(new ReplaceableParameter("$$type", page1_.isMultiplayer() ? "campaign_mp" : "campaign"));

		return TemplateProvider.getInstance().getProcessedTemplate(templateName, params);
	}
}

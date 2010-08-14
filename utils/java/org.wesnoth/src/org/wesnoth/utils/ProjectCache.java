/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import org.eclipse.core.resources.IProject;
import org.eclipse.jface.dialogs.DialogSettings;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.wesnoth.Logger;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.wml.core.Variable;
import org.wesnoth.wml.core.scenarios.Scenario;

/**
 * A class that stores some project specific infos
 * for current session.
 * Some of the fields of this cache can be saved to disk
 *  @see ProjectCache#saveCache()
 */
public class ProjectCache
{
	private IProject associatedProject_;

	private long propertiesTimetamp_;
	private long definesTimetamp_;

	private DialogSettings properties_;

	private File wesnothFile_;
	private File definesFile_;

	private Map<String, Scenario> scenarios_;
	private Map<String, Define> defines_;

	public ProjectCache(IProject project)
	{
		scenarios_ = new HashMap<String, Scenario>();
		defines_ = new HashMap<String, Define>(0);
		propertiesTimetamp_ = 0;
		properties_ = new DialogSettings("project");

		wesnothFile_ = new File(project.getLocation().toOSString()  +
							"/.wesnoth");
		definesFile_ = new File (PreprocessorUtils.getInstance().getTemporaryLocation(
				project.getFile("_main.cfg"))  + "/_MACROS_.cfg");

		ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());
		readProperties(true);
		readDefines(true);
	}

	/**
	 * Reads the properties from the file only if the
	 * file changed.
	 * @param force True to skip checking for modifications
	 */
	private void readProperties(boolean force)
	{
		if (force == false &&
			wesnothFile_.lastModified() <= propertiesTimetamp_)
			return;
		try
		{
			if (wesnothFile_.exists() == false)
				ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());

			try
			{
				properties_.load(wesnothFile_.getAbsolutePath());
			}
			catch(ClassCastException ex)
			{
				// backward compatiblity
				// we already have an xml format used by Properties.
				// convert it to DialogSettings
				ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());
				properties_.load(wesnothFile_.getAbsolutePath());
			}

			for(IDialogSettings scenario : properties_.getSection("scenarios").getSections())
			{
				if (scenario.getName().startsWith("scenario") == false)
					continue;

				Scenario tmp = new Scenario(scenario.get("filename"), scenario.get("id"));
				for(IDialogSettings variable : scenario.getSection("variables").getSections())
				{
					if (variable.getName().startsWith("var") == false)
						continue;
					tmp.getVariables().add(
							new Variable(variable.get("name"),
										variable.get("location"),
										variable.getInt("offset")));
				}
				scenarios_.put(scenario.get("filename"), tmp);
			}

			propertiesTimetamp_ = wesnothFile_.lastModified();
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			propertiesTimetamp_ = 0; // force to re-read the file
		}
	}

	public IProject getAssociatedProject()
	{
		return associatedProject_;
	}

	/**
	 * Gets the properties store for the associated Project.
	 * If the store doesn't exist it will be created.
	 * This method ensures it will get the latest up-to-date '.wesnoth' file
	 * @return
	 */
	public DialogSettings getProperties()
	{
		readProperties(false);
		return properties_;
	}

	/**
	 * Gets the map with the scenarios
	 * The key represent the filenames of the files
	 * and the value the scenarioId from that file
	 * @return
	 */
	public Map<String, Scenario> getScenarios()
	{
		return scenarios_;
	}

	/**
	 * Gets the scenario by it's filename.
	 * If the scenario doesn't exist it will be created
	 * @param name
	 * @return
	 */
	public Scenario getScenario(String name)
	{
		if (scenarios_.containsKey(name) == false)
			scenarios_.put(name, new Scenario(name, null));
		return scenarios_.get(name);
	}

	/**
	 * Saves the cache to disk.
	 * Saves:
	 * - properties
	 * - existing scenarios
	 * @return
	 */
	public boolean saveCache()
	{
		if (wesnothFile_.exists() == false)
			ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());
		try
		{
			// save scenario info
			int scenCnt = 0;
			IDialogSettings scenarios = properties_.addNewSection("scenarios");
			for(Scenario scenario : scenarios_.values())
			{
				IDialogSettings scenSection = scenarios.addNewSection("scenario" + scenCnt);
				scenSection.put("id", scenario.getId());
				scenSection.put("filename", scenario.getFilename());

				IDialogSettings variablesSection = scenSection.addNewSection("variables");
				int varCnt = 0;
				for(Variable var : scenario.getVariables())
				{
					IDialogSettings varSection = variablesSection.addNewSection("var" + varCnt);
					varSection.put("name", var.getName());
					varSection.put("location", var.getLocation());
					varSection.put("offset", var.getOffset());

					++varCnt;
				}
				++scenCnt;
			}

			// store properties
			properties_.save(wesnothFile_.getAbsolutePath());
			propertiesTimetamp_ = wesnothFile_.lastModified();
			return true;
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			return false;
		}
	}

	/**
	 * Reads the defines files for this project
	 * @param force skip checking for last modified timestamp
	 */
	public void readDefines(boolean force)
	{
		if (force == false &&
			definesFile_.lastModified() <= definesTimetamp_)
			return;
		if (definesFile_.exists() == false)
			return;
		defines_ = Define.readDefines(definesFile_.getAbsolutePath());
	}

	public void setDefines(Map<String, Define> defines)
	{
		defines_ = defines;
	}

	/**
	 * Returns the defines associated with this project
	 * @return
	 */
	public Map<String, Define> getDefines()
	{
		return defines_;
	}
}

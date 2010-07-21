/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.unit;

import java.util.ArrayList;

import wesnoth_eclipse_plugin.templates.ReplaceableParameter;
import wesnoth_eclipse_plugin.templates.TemplateProvider;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;

@Deprecated
public class UnitTypeWizard extends NewWizardTemplate
{

	private boolean			isFinished_	= false;
	private UnitTypePage0	page0_;

	public UnitTypeWizard() {
		setWindowTitle("Unittype tag wizard");
	}

	@Override
	public void addPages()
	{
		page0_ = new UnitTypePage0();
		addPage(page0_);

		super.addPages();
	}

	@Override
	public boolean isFinished()
	{
		return isFinished_;
	}

	@Override
	public boolean performFinish()
	{
		isFinished_ = true;
		ArrayList<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();

		objectName_ = page0_.getName();
		//params.add(new ReplaceableParameter("$$movetype_name", page0_.getName()));

		String template = TemplateProvider.getInstance().getProcessedTemplate("unit_type", params);

		if (template == null)
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Template for \"unit_type\" not found.");
			return false;
		}
		data_ = template;
		return true;
	}

}

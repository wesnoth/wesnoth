/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.campaign;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;

import wesnoth_eclipse_plugin.Activator;

public class CampaignPage0 extends WizardNewProjectCreationPage
{
	public CampaignPage0() {
		super("campaignPage0");
	}

	@Override
	public void createControl(Composite parent)
	{
		super.createControl(parent);
		setTitle("Campaign wizard");
		setMessage("Specify the name of the campaign project.");

		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(getShell(),
			"Wesnoth_Eclipse_Plugin.wizardHelp");
	}

	@Override
	public boolean canFlipToNextPage()
	{
		return super.canFlipToNextPage();
	}
}

/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.campaign;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;
import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;


public class CampaignPage0 extends WizardNewProjectCreationPage
{
	public CampaignPage0() {
		super("campaignPage0"); //$NON-NLS-1$
	}

	@Override
	public void createControl(Composite parent)
	{
		super.createControl(parent);
		setTitle(Messages.CampaignPage0_1);
		setMessage(Messages.CampaignPage0_2);

		WesnothPlugin.getDefault().getWorkbench().getHelpSystem().setHelp(getShell(),
			"org.wesnoth.wizardHelp"); //$NON-NLS-1$
	}
}

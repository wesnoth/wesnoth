/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.emptyproject;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;
import org.wesnoth.Activator;
import org.wesnoth.Messages;


public class EmptyProjectPage0 extends WizardNewProjectCreationPage
{
	public EmptyProjectPage0() {
		super("emptyProjectPage0"); //$NON-NLS-1$
	}

	@Override
	public void createControl(Composite parent)
	{
		super.createControl(parent);
		setTitle(Messages.EmptyProjectPage0_1);
		setMessage(Messages.EmptyProjectPage0_2);

		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(getShell(),
				"Wesnoth_Eclipse_Plugin.wizardHelp"); //$NON-NLS-1$
	}
}

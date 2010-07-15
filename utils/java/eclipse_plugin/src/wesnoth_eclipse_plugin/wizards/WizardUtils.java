/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Shell;

import wesnoth_eclipse_plugin.Activator;

public class WizardUtils
{
	public static int launchWizard(NewWizardTemplate wizard, Shell shell, IStructuredSelection selection)
	{
		if (wizard == null)
			return Window.CANCEL;

		wizard.init(Activator.getDefault().getWorkbench(), selection);
		wizard.setForcePreviousAndNextButtons(true);

		WizardDialog wizardDialog = new WizardDialog(shell, wizard);
		wizardDialog.create();
		//TODO: center wizard?
		wizardDialog.getShell().setLocation(shell.getBounds().x, shell.getBounds().y);
		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(wizardDialog.getShell(),
				"org.eclipse.ui.new_wizard_context");

		return wizardDialog.open();
	}
}

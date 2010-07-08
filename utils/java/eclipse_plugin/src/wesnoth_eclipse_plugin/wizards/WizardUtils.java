/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Shell;

import wesnoth_eclipse_plugin.Activator;

public class WizardUtils
{
	public static void launchWizard(NewWizardTemplate wizard, Shell shell, IStructuredSelection selection)
	{
		wizard.init(Activator.getDefault().getWorkbench(), selection);
		wizard.setForcePreviousAndNextButtons(true);

		WizardDialog wizardDialog = new WizardDialog(shell, wizard);
		wizardDialog.create();
		wizardDialog.getShell().setLocation(shell.getBounds().x, shell.getBounds().y);
		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(wizardDialog.getShell(),
				"org.eclipse.ui.new_wizard_context");

		wizardDialog.open();
	}
}

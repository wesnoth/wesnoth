/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import org.eclipse.jface.wizard.WizardDialog;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;

public class WizardLauncher extends NewWizardTemplate
{
	WizardLauncherPage0	page0_;

	public WizardLauncher() {
		setWindowTitle("Wizard launcher");
		page0_ = new WizardLauncherPage0();
		addPage(page0_);
	}

	@Override
	public void addPages()
	{
		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		WizardGenerator wizard = new WizardGenerator(page0_.getTagDescription() + " new wizard", page0_.getTagName());
		wizard.init(Activator.getDefault().getWorkbench(), selection_);
		wizard.setForcePreviousAndNextButtons(true);

		WizardDialog wizardDialog = new WizardDialog(getShell(), wizard);
		wizardDialog.create();
		wizardDialog.getShell().setLocation(getShell().getBounds().x, getShell().getBounds().y);
		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(wizardDialog.getShell(),
				"org.eclipse.ui.new_wizard_context");

		wizardDialog.open();
		return false;
	}
}

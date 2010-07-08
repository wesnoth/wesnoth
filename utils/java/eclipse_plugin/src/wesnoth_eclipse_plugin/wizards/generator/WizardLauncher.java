/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.WizardUtils;

public class WizardLauncher extends NewWizardTemplate
{
	WizardLauncherPage0	page0_;
	WizardLauncherPage1	page1_;

	public WizardLauncher() {
		setWindowTitle("Wizard launcher");
		setNeedsProgressMonitor(true);
	}

	@Override
	public void addPages()
	{
		page0_ = new WizardLauncherPage0(selection_);
		addPage(page0_);

		page1_ = new WizardLauncherPage1();
		addPage(page1_);

		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		WizardGenerator wizard = new WizardGenerator(page1_.getTagDescription() + " new wizard", page1_.getTagName());
		WizardUtils.launchWizard(wizard, getShell(), selection_);

		return false;
	}
}

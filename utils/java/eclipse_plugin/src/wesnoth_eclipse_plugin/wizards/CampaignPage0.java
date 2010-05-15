package wesnoth_eclipse_plugin.wizards;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;

public class CampaignPage0 extends WizardNewProjectCreationPage {

	public CampaignPage0() {
		super("wizardPage");
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.dialogs.WizardNewProjectCreationPage#createControl(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	public void createControl(Composite parent) {
		super.createControl(parent);
		setMessage("Specify the name of the campaign project.");
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.wizard.WizardPage#canFlipToNextPage()
	 */
	@Override
	public boolean canFlipToNextPage() {
		return super.canFlipToNextPage();
	}
}

/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.Composite;

public class NewWizardPageTemplate extends WizardPage
{
	protected NewWizardPageTemplate(String pageName)
	{
		super(pageName);
	}

	@Override
	public void createControl(Composite parent)
	{
	}

	@Override
	public NewWizardTemplate getWizard()
	{
		return (NewWizardTemplate)super.getWizard();
	}
}

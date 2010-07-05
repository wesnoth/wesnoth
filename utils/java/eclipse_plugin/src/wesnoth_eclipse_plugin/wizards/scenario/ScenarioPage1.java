/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.wizards.scenario;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;

public class ScenarioPage1 extends WizardPage {

	/**
	 * Create the wizard.
	 */
	public ScenarioPage1() {
		super("wizardPage");
		setTitle("Scenario file");
		setDescription("Set scenario details");
	}

	/**
	 * Create contents of the wizard.
	 * @param parent
	 */
	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
	}

}

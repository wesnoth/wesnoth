/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.HashMap;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

public class WizardLauncherPage0 extends WizardPage
{
	private HashMap<String, String>	list_;
	private Combo					cmbWizardName_;

	public WizardLauncherPage0() {
		super("wizardPage");
		setTitle("Wizard Launcher");
		setDescription("Launch a wizard");
		list_ = new HashMap<String, String>();
		list_.put("Campaign", "campaign");
		list_.put("Game config", "game_config");
		list_.put("AIs", "ais");
		//TODO: read the list-tag from file!
	}

	@Override
	public void createControl(Composite parent)
	{
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		container.setLayout(new GridLayout(2, false));

		Label lblSelectAWizard = new Label(container, SWT.NONE);
		lblSelectAWizard.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
		lblSelectAWizard.setText("Select a Wizard: ");

		cmbWizardName_ = new Combo(container, SWT.NONE);
		cmbWizardName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		String[] items = new String[0];
		cmbWizardName_.setItems(list_.keySet().toArray(items));
	}

	public String getTagName()
	{
		return list_.get(cmbWizardName_.getText());
	}

	public String getTagDescription()
	{
		return cmbWizardName_.getText();
	}
}

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

public class WizardLauncherPage1 extends WizardPage
{
	private HashMap<String, String>	list_;
	private Combo					cmbWizardName_;

	public WizardLauncherPage1() {
		super("wizardLauncherPage1");
		setTitle("Wizard Launcher");
		setDescription("Select wizard to launch");
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
		container.setLayout(new GridLayout(3, false));
		String[] items = new String[0];
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		Label label = new Label(container, SWT.NONE);
		GridData gd_label = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
		gd_label.widthHint = 141;
		label.setLayoutData(gd_label);
		label.setText("    ");
		new Label(container, SWT.NONE);

		Label label_1 = new Label(container, SWT.NONE);
		GridData gd_label_1 = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
		gd_label_1.widthHint = 146;
		label_1.setLayoutData(gd_label_1);
		label_1.setText(" ");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		Label lblSelectAWizard = new Label(container, SWT.NONE);
		lblSelectAWizard.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblSelectAWizard.setText("Select a Wizard and then press finish: ");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		cmbWizardName_ = new Combo(container, SWT.READ_ONLY);
		cmbWizardName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		cmbWizardName_.setItems(list_.keySet().toArray(items));
		cmbWizardName_.select(0);
		new Label(container, SWT.NONE);
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

/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import com.swtdesigner.SWTResourceManager;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

public class WizardGeneratorPage404 extends WizardPage
{
	public WizardGeneratorPage404(String tag) {
		super("wizardGeneratorPage404");
		setErrorMessage("content not found for tag '" + tag + "'");
		setTitle("404 Not Found");
		setDescription("Ooops!");
	}

	@Override
	public void createControl(Composite parent)
	{
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);

		Label lblThisIsSooo = new Label(container, SWT.WRAP);
		lblThisIsSooo.setFont(SWTResourceManager.getFont("Segoe UI", 13, SWT.NORMAL));
		lblThisIsSooo.setBounds(82, 65, 415, 132);
		lblThisIsSooo.setText("This is embarassing. It shouldn't happen. \r\nIt seems something is missing from the schema.cfg");
	}
}

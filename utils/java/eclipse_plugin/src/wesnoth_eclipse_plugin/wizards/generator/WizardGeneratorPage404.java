/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
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

		Font font = new Font(Display.getDefault().getSystemFont().getDevice(),
				Display.getDefault().getSystemFont().getFontData()[0].getName(), 20, SWT.NORMAL);
		Label lblThisIsSooo = new Label(container, SWT.WRAP);
		lblThisIsSooo.setFont(font);
		lblThisIsSooo.setBounds(10, 89, 554, 137);
		lblThisIsSooo.setText("This is embarassing. It shouldn't happen. \r\nIt seems something is missing from schema.cfg");
	}
}

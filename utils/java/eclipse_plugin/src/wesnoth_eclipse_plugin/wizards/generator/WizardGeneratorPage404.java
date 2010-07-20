/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.generator;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;

import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class WizardGeneratorPage404 extends NewWizardPageTemplate
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
		super.createControl(parent);
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);

		Font font = new Font(Display.getDefault().getSystemFont().getDevice(),
				Display.getDefault().getSystemFont().getFontData()[0].getName(), 20, SWT.NORMAL);
		Label lblThisIsSooo = new Label(container, SWT.WRAP);
		lblThisIsSooo.setFont(font);
		lblThisIsSooo.setBounds(10, 89, 554, 137);
		lblThisIsSooo.setText("It seems something is missing from schema.cfg.");
	}
}

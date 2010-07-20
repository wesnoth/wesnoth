/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.scenario;

import java.util.Properties;

import org.eclipse.core.resources.IContainer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.utils.ProjectUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class ScenarioPage1 extends NewWizardPageTemplate
{
	private Composite container_;

	public ScenarioPage1() {
		super("scenarioPage1");
		setTitle("Scenario file");
		setDescription("Set scenario details");
	}

	@Override
	public void createControl(Composite parent)
	{
		container_ = new Composite(parent, SWT.NULL);

		setControl(container_);
		container_.setLayout(new GridLayout(4, false));

		Label lblSpecifyTheGold = new Label(container_, SWT.NONE);
		lblSpecifyTheGold.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 4, 1));
		lblSpecifyTheGold.setText("Specify the gold for each difficulty:");

		IContainer selContainer = getWizard().getSelectionContainer();
		if (selContainer != null)
		{
			Properties prefs =
				ProjectUtils.getPropertiesForProject(selContainer.getProject());
			if (prefs != null && prefs.getProperty("difficulties") != null)
			{
				String[] difficulties = prefs.getProperty("difficulties").split(",");
				for (String diff : difficulties)
				{
					if (diff.isEmpty())
						continue;

					Label label = new Label(container_, SWT.NONE);
					label.setText("    ");

					Label lblDiff = new Label(container_, SWT.NONE);
					lblDiff.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
					lblDiff.setText(diff + ":");

					Text textBox = new Text(container_, SWT.BORDER);
					GridData gd_text = new GridData(SWT.LEFT, SWT.CENTER, true, false, 1, 1);
					gd_text.widthHint = 77;
					textBox.setData("diff",diff);
					textBox.setLayoutData(gd_text);
					new Label(container_, SWT.NONE);
				}
			}
		}
	}

	/**
	 * Gets the starting gold as #ifdef based on difficulties
	 */
	public String getStartingGoldByDifficulties()
	{
		StringBuilder result = new StringBuilder();
		for(Control control: container_.getChildren())
		{
			if (!(control instanceof Text))
				continue;
			Text textBox = (Text)control;
			result.append(String.format("#ifdef %s\n\tgold=%s\n#endif\n",
					textBox.getData("diff").toString(), textBox.getText()));
		}
		return result.toString();
	}

}

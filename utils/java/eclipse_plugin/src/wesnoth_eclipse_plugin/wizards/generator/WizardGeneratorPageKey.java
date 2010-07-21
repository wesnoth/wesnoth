/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.schema.TagKey;
import wesnoth_eclipse_plugin.utils.StringUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class WizardGeneratorPageKey extends NewWizardPageTemplate
{
	private List<TagKey>	keys_;
	private int				startIndex_, endIndex_;
	private Composite		container_;
	private byte			indent_;

	public WizardGeneratorPageKey(String tagName, List<TagKey> keys,
			int startIndex, int endIndex, byte indent) {
		super("wizardPageKey" + startIndex);
		setTitle(tagName + " new wizard");
		//setDescription(String.format("page %d to %d out of %d", startIndex, endIndex, keys.size()));

		indent_ = indent;

		startIndex_ = startIndex;
		endIndex_ = endIndex;
		keys_ = keys;
	}

	@Override
	public void createControl(Composite parent)
	{
		super.createControl(parent);
		container_ = new Composite(parent, SWT.NULL);
		setControl(container_);
		container_.setLayout(new GridLayout(2, false));

		for (int i = startIndex_; i <= endIndex_; i++)
		{
			TagKey key = keys_.get(i);

			if (key.Cardinality == '-')
				continue;

			Label label = new Label(container_, SWT.NONE);
			label.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			// add star to required items
			label.setText(key.Name + (key.Cardinality == '1' ? "*" : "") + ":");

			// if the is an enum create a combobox instead of textbox
			if (key.IsEnum)
			{
				Combo combo = new Combo(container_, SWT.READ_ONLY);
				combo.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
				combo.setData("name", key.Name);
				String[] items = key.ValueType.split(",");
				combo.setItems(items);
				combo.select(0);
			}
			else
			{
				Text textBox = new Text(container_, SWT.BORDER);
				textBox.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

				textBox.setData("name", key.Name);
				textBox.setData("valType", key.ValueType);
				textBox.setData("card", key.Cardinality);
				textBox.setData("trans", key.IsTranslatable);
				if (key.Cardinality == '1')
					textBox.setData("comp", false); // is textbox complete

				textBox.addModifyListener(new ModifyListener() {
					@Override
					public void modifyText(ModifyEvent e)
				{
					if (!(e.getSource() instanceof Text))
						return;

					Text txt = ((Text) e.getSource());
					if (txt.getData("comp") == null)
						return;

					if ((txt.getText().isEmpty() && (txt.getData("card").toString().equals("1"))) || // cardinality
						!txt.getText().matches(txt.getData("valType").toString()) // regex
					)
						txt.setData("comp", false);
					else
						txt.setData("comp", true);
					updatePageIsComplete();
				}
				});
			}
		}
		updatePageIsComplete();
	}

	private void updatePageIsComplete()
	{
		setPageComplete(false);

		for (Control child : container_.getChildren())
		{
			if (!(child instanceof Text)) // don't check comboboxes
				continue;

			if (child.getData("comp") == null)
				continue;

			if (child.getData("comp").toString().equals("false"))
			{
				setErrorMessage(child.getData("name") +
						" is empty or does not match required value\n"
						+ child.getData("valType"));
				return;
			}
		}

		setPageComplete(true);
		setErrorMessage(null);
	}

	public String getContent()
	{
		StringBuilder result = new StringBuilder();
		for (Control child : container_.getChildren())
		{
			if (!(child instanceof Text || child instanceof Combo))
				continue;
			String text = "";
			if (child instanceof Text)
				text = (child.getData("trans").toString().equals("true") == true ? "_" : "") +
						"\"" + ((Text) child).getText() + "\"";
			else
				text = ((Combo) child).getText();
			result.append(StringUtils.multiples("\t", indent_) +
								child.getData("name") + "=" + text + "\n");
		}
		return result.toString();
	}
}

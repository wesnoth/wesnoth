/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map.Entry;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.List;

import wesnoth_eclipse_plugin.schema.Tag;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ListUtils;
import wesnoth_eclipse_plugin.utils.StringUtils;
import wesnoth_eclipse_plugin.utils.WizardUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class WizardGeneratorPageTag extends NewWizardPageTemplate
{
	private java.util.List<Tag>						tags_;
	private HashMap<String, java.util.List<String>>	content_;
	private int									startIndex_, endIndex_;
	private Composite								container_;
	private byte									indent_;

	public WizardGeneratorPageTag(String tagName, java.util.List<Tag> tags,
			int startIndex, int endIndex, byte indent) {
		super("wizardPageTag" + startIndex);
		setTitle(tagName + " new wizard");
		//setDescription(String.format("page %d to %d out of %d", startIndex, endIndex, tags.size()));

		indent_ = indent;

		startIndex_ = startIndex;
		endIndex_ = endIndex;
		tags_ = tags;
		content_ = new HashMap<String, java.util.List<String>>();
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
			final Tag tag = tags_.get(i);
			if (tag.isForbidden())
				continue;

			Group tagGroup = new Group(container_, SWT.NONE);
			tagGroup.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			tagGroup.setText("[" + tag.getName() + "]");
			tagGroup.setLayout(new GridLayout(2, false));

			List list = new List(tagGroup, SWT.BORDER);
			GridData gd_list = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 2);
			gd_list.heightHint = 81;
			gd_list.widthHint = 150;
			list.setLayoutData(gd_list);

			Button btnAdd = new Button(tagGroup, SWT.NONE);
			GridData gd_btnAdd = new GridData(SWT.FILL, SWT.FILL, false, false, 1, 1);
			gd_btnAdd.heightHint = 40;
			btnAdd.setLayoutData(gd_btnAdd);
			btnAdd.setText("Add");
			btnAdd.setData("list", list);
			btnAdd.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					if (!(e.getSource() instanceof Button))
						return;
					addNewItem((List) ((Button) e.getSource()).getData("list"), tag);
				}
			});

			Button btnRemove = new Button(tagGroup, SWT.NONE);
			btnRemove.setLayoutData(new GridData(SWT.FILL, SWT.FILL, false, false, 1, 1));
			btnRemove.setText("Remove");
			btnRemove.setData("list", list);
			btnRemove.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					if (!(e.getSource() instanceof Button))
						return;
					removeItem((List) ((Button) e.getSource()).getData("list"), tag.getName());
				}
			});

			tagGroup.setData("list", list);
			tagGroup.setData("tag", tag);
			content_.put(tag.getName(), new ArrayList<String>());
		}
		updatePageIsComplete();
	}

	private void addNewItem(List targetList, Tag tag)
	{
		if ((tag.isOptional() || tag.isRequired()) &&
			targetList.getItemCount() == 1)
		{
			GUIUtils.showWarnMessageBox("You can't add more than one item.");
			return;
		}

		WizardGenerator wizard =
				new WizardGenerator("Create a new " + tag.getName(), tag.getName(),
						(byte) (indent_ + 1));
		WizardUtils.launchWizard(wizard, getShell(), null);
		if (wizard.isFinished())
		{
			targetList.add(wizard.getObjectName() + targetList.getItemCount());
			content_.get(tag.getName()).add(wizard.getData().toString());
		}
		updatePageIsComplete();
	}

	private void removeItem(List targetList, String tagName)
	{
		if (targetList.getSelectionCount() == 0 || targetList.getItemCount() == 0)
		{
			GUIUtils.showWarnMessageBox("Please select an item before removing it.");
			return;
		}

		content_.get(tagName).remove(targetList.getSelectionIndex());
		targetList.remove(targetList.getSelectionIndex());
		updatePageIsComplete();
	}

	private void updatePageIsComplete()
	{
		setPageComplete(false);
		for(Control control : container_.getChildren())
		{
			if (!(control instanceof Group))
				continue;

			int cnt = ((List)control.getData("list")).getItemCount();
			Tag tag = (Tag)control.getData("tag");
			if (cnt == 0 && tag.isRequired())
			{
				setErrorMessage("You need to have a [" + tag.getName() + "] defined.");
				return;
			}
		}

		setPageComplete(true);
		setErrorMessage(null);
	}

	public String getContent()
	{
		StringBuilder result = new StringBuilder();
		for (Entry<String, java.util.List<String>> tag : content_.entrySet())
		{
			result.append(StringUtils.multiples("\t", indent_) + "[" + tag.getKey() + "]\n");
			result.append(ListUtils.concatenateList(tag.getValue(), "\n\t"));
			result.append(StringUtils.multiples("\t", indent_) + "[/" + tag.getKey() + "]\n");
		}
		return result.toString();
	}
}

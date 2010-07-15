/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.unit;

import java.util.ArrayList;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.List;

import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ListUtils;

@Deprecated
public class UnitTypePage1 extends WizardPage
{
	private List			lstAttack_;
	private List			lstAdvancement_;

	java.util.List<String>	attacks_;
	java.util.List<String>	advancements_;

	public UnitTypePage1() {
		super("unitTypePage1");
		setTitle("Unit type wizard");
		setDescription("Unit type details");
		attacks_ = new ArrayList<String>();
		advancements_ = new ArrayList<String>();
	}

	@Override
	public void createControl(Composite parent)
	{
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		container.setLayout(new GridLayout(2, false));

		Group grpattack = new Group(container, SWT.NONE);
		grpattack.setLayout(null);
		grpattack.setText("[attack]");

		lstAttack_ = new List(grpattack, SWT.BORDER);
		lstAttack_.setBounds(4, 25, 190, 68);
		lstAttack_.setData(attacks_);

		Button addAttack = new Button(grpattack, SWT.NONE);
		addAttack.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				addItem(lstAttack_, "attack");
			}
		});
		addAttack.setText("Add");
		addAttack.setBounds(200, 24, 65, 25);

		Button RemoveAttack = new Button(grpattack, SWT.NONE);
		RemoveAttack.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				removeItem(lstAttack_);
			}
		});
		RemoveAttack.setText("Remove");
		RemoveAttack.setBounds(200, 68, 65, 25);

		Group grpadvancement = new Group(container, SWT.NONE);
		grpadvancement.setLayout(null);
		grpadvancement.setText("[advancement]");

		lstAdvancement_ = new List(grpadvancement, SWT.BORDER);
		lstAdvancement_.setBounds(4, 25, 190, 68);
		lstAdvancement_.setData(advancements_);

		Button addAdvancement = new Button(grpadvancement, SWT.NONE);
		addAdvancement.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				addItem(lstAdvancement_, "advancement");
			}
		});
		addAdvancement.setText("Add");
		addAdvancement.setBounds(200, 24, 65, 25);

		Button btnRemoveAdvancement = new Button(grpadvancement, SWT.NONE);
		btnRemoveAdvancement.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				removeItem(lstAdvancement_);
			}
		});
		btnRemoveAdvancement.setText("Remove");
		btnRemoveAdvancement.setBounds(200, 68, 65, 25);
		setPageComplete(true);
	}

	private void addItem(List targetList, String type)
	{

	}

	@SuppressWarnings("unchecked")
	private void removeItem(List targetList)
	{
		if (targetList.getItemCount() == 0 || targetList.getSelectionCount() == 0)
		{
			GUIUtils.showMessageBox("Please select an item before trying to remove it.");
			return;
		}
		((java.util.List<String>) targetList.getData()).remove(targetList.getSelectionIndex());
		targetList.remove(targetList.getSelectionIndex());
	}

	public String getAdvancements()
	{
		return ListUtils.concatenateList(advancements_, "\n\t");
	}

	public String getAttacks()
	{
		return ListUtils.concatenateList(attacks_, "\n\t");
	}
}

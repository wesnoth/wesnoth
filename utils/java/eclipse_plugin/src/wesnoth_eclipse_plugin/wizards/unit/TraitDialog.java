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

import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

public class TraitDialog extends TitleAreaDialog
{
	private Text			txtId_;
	private Text			txtName_;
	private Text			txtMaleName_;
	private Text			txtFemaleName_;
	private Text			txtDescription_;
	private Combo			txtAvailability;

	java.util.List<String>	effects_;
	private List			lstEffect;
	private String			traitId_;

	public TraitDialog(Shell parentShell) {
		super(parentShell);
		effects_ = new ArrayList<String>();
	}

	@Override
	protected Control createDialogArea(Composite parent)
	{
		setTitle("Create a new trait");
		Composite area = (Composite) super.createDialogArea(parent);
		Composite container = new Composite(area, SWT.NONE);
		container.setLayout(new GridLayout(4, false));
		container.setLayoutData(new GridData(GridData.FILL_BOTH));

		Label lblId = new Label(container, SWT.NONE);
		lblId.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblId.setText("Id*:");

		txtId_ = new Text(container, SWT.BORDER);
		txtId_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblAvailability = new Label(container, SWT.NONE);
		lblAvailability.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
		lblAvailability.setText("Availability:");

		txtAvailability = new Combo(container, SWT.READ_ONLY);
		txtAvailability.setItems(new String[] { "", "musthave", "any", "none" });
		txtAvailability.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblName = new Label(container, SWT.NONE);
		lblName.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblName.setText("Name:");

		txtName_ = new Text(container, SWT.BORDER);
		txtName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblMaleName = new Label(container, SWT.NONE);
		lblMaleName.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblMaleName.setText("Male name:");

		txtMaleName_ = new Text(container, SWT.BORDER);
		txtMaleName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblFemaleName = new Label(container, SWT.NONE);
		lblFemaleName.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblFemaleName.setText("Female name:");

		txtFemaleName_ = new Text(container, SWT.BORDER);
		txtFemaleName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblDescription = new Label(container, SWT.NONE);
		lblDescription.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblDescription.setText("Description:");

		txtDescription_ = new Text(container, SWT.BORDER);
		txtDescription_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lbleffect = new Label(container, SWT.NONE);
		lbleffect.setLayoutData(new GridData(SWT.CENTER, SWT.CENTER, false, false, 1, 2));
		lbleffect.setText("[effect]");

		lstEffect = new List(container, SWT.BORDER);
		lstEffect.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 2));

		Button btnAdd = new Button(container, SWT.NONE);
		btnAdd.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				addEffect();
			}
		});
		btnAdd.setText("Add");
		new Label(container, SWT.NONE);

		Button btnRemove = new Button(container, SWT.NONE);
		btnRemove.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				removeEffect();
			}
		});
		btnRemove.setText("Remove");
		new Label(container, SWT.NONE);

		return area;
	}

	private void addEffect()
	{
		EffectDialog dialog = new EffectDialog(getShell());
		if (dialog.open() == Window.OK)
		{
			// add the effect
		}
	}

	private void removeEffect()
	{
		if (lstEffect.getSelectionCount() == 0 || lstEffect.getItems().length == 0)
			return;
		effects_.remove(lstEffect.getSelectionCount());
		lstEffect.remove(lstEffect.getSelectionIndex());
	}

	@Override
	protected void createButtonsForButtonBar(Composite parent)
	{
		createButton(parent, IDialogConstants.OK_ID, IDialogConstants.OK_LABEL, true);
		createButton(parent, IDialogConstants.CANCEL_ID, IDialogConstants.CANCEL_LABEL, false);
	}

	@Override
	protected Point getInitialSize()
	{
		return new Point(450, 337);
	}

	@Override
	public boolean close()
	{
		traitId_ = txtId_.getText();
		return super.close();
	}

	public void updateIsComplete()
	{
		getButton(IDialogConstants.OK_ID).setEnabled(false);

		// validation
		setErrorMessage(null);
		getButton(IDialogConstants.OK_ID).setEnabled(true);
	}

	public String getTrait()
	{
		String result = traitId_;
		// process the template
		return result;
	}

	public String getTraitId()
	{
		return traitId_;
	}
}

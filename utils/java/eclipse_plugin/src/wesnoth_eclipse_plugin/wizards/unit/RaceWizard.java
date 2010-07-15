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

import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ListUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class RaceWizard extends NewWizardTemplate
{

	private boolean		isFinished_	= false;
	private RacePage0	page0_;

	public RaceWizard() {
		setWindowTitle("Race tag wizard");
	}

	@Override
	public void addPages()
	{
		page0_ = new RacePage0();
		addPage(page0_);

		super.addPages();
	}

	@Override
	public boolean isFinished()
	{
		return isFinished_;
	}

	@Override
	public boolean performFinish()
	{
		isFinished_ = true;
		ArrayList<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();

		objectName_ = page0_.getName();

		params.add(new ReplaceableParameter("$$race_id", page0_.getRaceId()));
		params.add(new ReplaceableParameter("$$plural_name", page0_.getPluralName()));
		params.add(new ReplaceableParameter("$$male_name", page0_.getMaleName()));
		params.add(new ReplaceableParameter("$$female_name", page0_.getFemaleName()));
		params.add(new ReplaceableParameter("$$description", page0_.getRaceDescription()));
		params.add(new ReplaceableParameter("$$num_traits", String.valueOf(page0_.getTraitsNumber())));
		params.add(new ReplaceableParameter("$$ignore_global_traits", String.valueOf(page0_.getIgnoreGlobalTraits())));
		params.add(new ReplaceableParameter("$$traits", page0_.getTraits()));

		String template = TemplateProvider.getInstance().getProcessedTemplate("race", params);

		if (template == null)
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Template for \"race\" not found.");
			return false;
		}
		data_ = template;
		return true;
	}

	public class RacePage0 extends WizardPage
	{
		private List					lstTrait_;
		private Button					btnAddTrait;
		private Button					btnRemoveTrait;
		private Text					txtId_;
		private Text					txtPluralName_;
		private Text					txtMaleName_;
		private Text					txtFemaleName_;
		private Button					chkIgnoreGlobalTraits;
		private Spinner					txtTraitsNumber_;
		private Text					txtRaceName_;

		private java.util.List<String>	traitsList_;
		private Label					lblDescription;
		private Text					txtDescription_;

		public RacePage0() {
			super("racePage0");
			setTitle("Race wizard");
			setDescription("Create a new [race]");
			traitsList_ = new ArrayList<String>();
		}

		@Override
		public void createControl(Composite parent)
		{
			ModifyListener modifyListener = new ModifyListener() {
				@Override
				public void modifyText(ModifyEvent e)
				{
					updatePageIsComplete();
				}
			};
			Composite container = new Composite(parent, SWT.NULL);

			setControl(container);
			container.setLayout(new GridLayout(4, false));

			Label lblId = new Label(container, SWT.NONE);
			lblId.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			lblId.setText("Id*:");

			txtId_ = new Text(container, SWT.BORDER);
			txtId_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 3, 1));
			txtId_.addModifyListener(modifyListener);

			Label lblPluralname = new Label(container, SWT.NONE);
			lblPluralname.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			lblPluralname.setText("Plural_name*:");

			txtPluralName_ = new Text(container, SWT.BORDER);
			GridData gd_txtPluralName_ = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
			gd_txtPluralName_.widthHint = 211;
			txtPluralName_.setLayoutData(gd_txtPluralName_);
			txtPluralName_.addModifyListener(modifyListener);

			Label lblName_3 = new Label(container, SWT.NONE);
			lblName_3.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			lblName_3.setText("Name:");

			txtRaceName_ = new Text(container, SWT.BORDER);
			txtRaceName_.addModifyListener(new ModifyListener() {
				@Override
				public void modifyText(ModifyEvent e)
				{
					if (!(e.getSource() instanceof Text))
						return;
					boolean otherNamesEnabled = false;
					if (((Text) e.getSource()).getText().isEmpty())
					{
						otherNamesEnabled = true;
					}
					txtFemaleName_.setEnabled(otherNamesEnabled);
					txtMaleName_.setEnabled(otherNamesEnabled);

				}
			});
			txtRaceName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

			Label lblName_1 = new Label(container, SWT.NONE);
			lblName_1.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			lblName_1.setText("Male Name:");

			txtMaleName_ = new Text(container, SWT.BORDER);
			txtMaleName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));

			Label lblFemaleName = new Label(container, SWT.NONE);
			lblFemaleName.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			lblFemaleName.setText("Female name:");

			txtFemaleName_ = new Text(container, SWT.BORDER);
			txtFemaleName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

			chkIgnoreGlobalTraits = new Button(container, SWT.CHECK);
			chkIgnoreGlobalTraits.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 2, 1));
			chkIgnoreGlobalTraits.setText("Ignore global traits");

			Label lblTraitsNumber = new Label(container, SWT.NONE);
			lblTraitsNumber.setText("Traits number:");

			txtTraitsNumber_ = new Spinner(container, SWT.BORDER);

			lblDescription = new Label(container, SWT.NONE);
			lblDescription.setText("Description:");

			txtDescription_ = new Text(container, SWT.BORDER | SWT.MULTI);
			GridData gd_txtDescription_ = new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1);
			gd_txtDescription_.heightHint = 51;
			txtDescription_.setLayoutData(gd_txtDescription_);

			Group grptrait = new Group(container, SWT.NONE);
			grptrait.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 2, 1));
			grptrait.setLayout(null);
			grptrait.setText("[trait]");

			lstTrait_ = new List(grptrait, SWT.BORDER);
			lstTrait_.setBounds(4, 25, 190, 68);

			btnAddTrait = new Button(grptrait, SWT.NONE);
			btnAddTrait.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					addNewItem();
				}
			});
			btnAddTrait.setText("Add");
			btnAddTrait.setBounds(200, 24, 65, 25);

			btnRemoveTrait = new Button(grptrait, SWT.NONE);
			btnRemoveTrait.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					removeSelectedItem();
				}
			});
			btnRemoveTrait.setText("Remove");
			btnRemoveTrait.setBounds(200, 68, 65, 25);
			new Label(container, SWT.NONE);
			new Label(container, SWT.NONE);
			updatePageIsComplete();
		}

		private void addNewItem()
		{
			TraitDialog dialog = new TraitDialog(getShell());
			if (dialog.open() == Window.OK)
			{
				lstTrait_.add(dialog.getTraitId());
				traitsList_.add(dialog.getTrait());
			}
		}

		private void removeSelectedItem()
		{
			if (lstTrait_ == null)
				return;
			if (lstTrait_.getSelectionCount() == 0 || lstTrait_.getItems().length == 0)
			{
				GUIUtils.showMessageBox("Please select a trait before deleting it.");
				return;
			}
			traitsList_.remove(lstTrait_.getSelectionIndex());
			lstTrait_.remove(lstTrait_.getSelectionIndex());
		}

		private void updatePageIsComplete()
		{
			setPageComplete(false);
			setErrorMessage(null);
			if (getRaceId().isEmpty())
			{
				setErrorMessage("The ID cannot be empty.");
				return;
			}
			if (getPluralName().isEmpty())
			{
				setErrorMessage("The plural_name cannot be empty.");
				return;
			}
			setPageComplete(true);
		}

		public boolean getIgnoreGlobalTraits()
		{
			return chkIgnoreGlobalTraits.getSelection();
		}

		public String getTraits()
		{
			return ListUtils.concatenateList(traitsList_, "\n\t");
		}

		public String getRaceId()
		{
			return txtId_.getText();
		}

		public String getMaleName()
		{
			return getRaceName().isEmpty() ? txtMaleName_.getText() : getRaceName();
		}

		public String getFemaleName()
		{
			return getRaceName().isEmpty() ? txtFemaleName_.getText() : getRaceName();
		}

		public String getRaceName()
		{
			return txtRaceName_.getText();
		}

		public String getPluralName()
		{
			return txtPluralName_.getText();
		}

		public int getTraitsNumber()
		{
			return txtTraitsNumber_.getSelection();
		}

		public String getRaceDescription()
		{
			return txtDescription_.getText();
		}
	}
}

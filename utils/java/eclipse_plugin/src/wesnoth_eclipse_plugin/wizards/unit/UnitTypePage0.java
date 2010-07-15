/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.unit;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;

@Deprecated
public class UnitTypePage0 extends WizardPage
{
	private Text	txtId_;
	private Text	txtUnitTypeName_;
	private Text	txtAdvancesTo_;
	private Text	txtCost;
	private Text	txtUsage_;
	private Text	txtExperience;
	private Text	txtHitpoints_;
	private Text	txtLevel_;
	private Text	txtMovement_;
	private Text	txtMovementType_;
	private Text	txtRace_;
	private Text	txtTraitsNumber_;
	private Text	txtImage_;
	private Text	txtProfile_;
	private Text	txtUndeadVariation_;
	private Combo	cmbZoc_;
	private Text	txtEllipse_;
	private Text	txtDieSound_;
	private Text	txtDecscription_;
	private Combo	cmbIgnoreTraits_;
	private Combo	cmbHideHelp_;
	private Combo	cmbAlignment_;
	private Text	txtBaseUnitId_;
	private Combo	cmbGender;
	private Spinner	txtAttacks;

	public UnitTypePage0() {
		super("unitTypePage0");
		setTitle("Unit type wizard");
		setDescription("Create a new [unit_type]");
	}

	@Override
	public void createControl(Composite parent)
	{
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		container.setLayout(new GridLayout(8, false));

		Label lblName = new Label(container, SWT.NONE);
		GridData gd_lblName = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_lblName.widthHint = 68;
		lblName.setLayoutData(gd_lblName);
		lblName.setText("Id*:");

		txtId_ = new Text(container, SWT.BORDER);
		GridData gd_txtId_ = new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1);
		gd_txtId_.widthHint = 190;
		txtId_.setLayoutData(gd_txtId_);

		Label lblName_1 = new Label(container, SWT.NONE);
		lblName_1.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblName_1.setText("Name:");

		txtUnitTypeName_ = new Text(container, SWT.BORDER);
		txtUnitTypeName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblAdvancesTo = new Label(container, SWT.NONE);
		lblAdvancesTo.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblAdvancesTo.setText("Advances to:");

		txtAdvancesTo_ = new Text(container, SWT.BORDER);
		GridData gd_txtAdvancesTo_ = new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1);
		gd_txtAdvancesTo_.widthHint = 190;
		txtAdvancesTo_.setLayoutData(gd_txtAdvancesTo_);

		Label lblAttacks = new Label(container, SWT.NONE);
		lblAttacks.setText("Attacks:");

		txtAttacks = new Spinner(container, SWT.BORDER);
		txtAttacks.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false, false, 1, 1));
		txtAttacks.setMinimum(1);

		Label lblGender = new Label(container, SWT.NONE);
		lblGender.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblGender.setText("Gender:");

		cmbGender = new Combo(container, SWT.READ_ONLY);
		cmbGender.setItems(new String[] { "Male", "Female" });
		cmbGender.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		cmbGender.select(0);

		Label lblCost = new Label(container, SWT.NONE);
		lblCost.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblCost.setText("Cost:");

		txtCost = new Text(container, SWT.BORDER);
		GridData gd_txtCost = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd_txtCost.widthHint = 68;
		txtCost.setLayoutData(gd_txtCost);

		Label lblExperience = new Label(container, SWT.NONE);
		lblExperience.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblExperience.setText("Experience:");

		txtExperience = new Text(container, SWT.BORDER);
		GridData gd_txtExperience = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd_txtExperience.widthHint = 52;
		txtExperience.setLayoutData(gd_txtExperience);

		Label lblUsage = new Label(container, SWT.NONE);
		lblUsage.setText("Usage:");

		txtUsage_ = new Text(container, SWT.BORDER);
		txtUsage_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblRace = new Label(container, SWT.NONE);
		lblRace.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblRace.setText("Race:");

		txtRace_ = new Text(container, SWT.BORDER);
		txtRace_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblHitpoints = new Label(container, SWT.NONE);
		lblHitpoints.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblHitpoints.setText("Hitpoints:");

		txtHitpoints_ = new Text(container, SWT.BORDER);
		txtHitpoints_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblLevel = new Label(container, SWT.NONE);
		lblLevel.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblLevel.setText("Level:");

		txtLevel_ = new Text(container, SWT.BORDER);
		GridData gd_txtLevel_ = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd_txtLevel_.widthHint = 51;
		txtLevel_.setLayoutData(gd_txtLevel_);

		Label lblMovement = new Label(container, SWT.NONE);
		lblMovement.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblMovement.setText("Movement:");

		txtMovement_ = new Text(container, SWT.BORDER);
		txtMovement_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblMovementType = new Label(container, SWT.NONE);
		lblMovementType.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblMovementType.setText("Movement type:");

		txtMovementType_ = new Text(container, SWT.BORDER);
		GridData gd_txtMovementType_ = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd_txtMovementType_.widthHint = 89;
		txtMovementType_.setLayoutData(gd_txtMovementType_);

		Label lblIgnoreGlobalTraits = new Label(container, SWT.NONE);
		lblIgnoreGlobalTraits.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
		lblIgnoreGlobalTraits.setText("Ignore race traits:");

		cmbIgnoreTraits_ = new Combo(container, SWT.READ_ONLY);
		cmbIgnoreTraits_.setItems(new String[] { "false", "true" });
		cmbIgnoreTraits_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		cmbIgnoreTraits_.select(0);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		Label lblTraitsNumber = new Label(container, SWT.NONE);
		lblTraitsNumber.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
		lblTraitsNumber.setText("Traits number:");

		txtTraitsNumber_ = new Text(container, SWT.BORDER);
		txtTraitsNumber_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		Label lblAlignment = new Label(container, SWT.NONE);
		lblAlignment.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblAlignment.setText("Alignment:");

		cmbAlignment_ = new Combo(container, SWT.READ_ONLY);
		cmbAlignment_.setItems(new String[] { "neutral", "chaotic", "lawful" });
		cmbAlignment_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		cmbAlignment_.select(0);

		Label lblImage = new Label(container, SWT.NONE);
		lblImage.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblImage.setText("Image:");

		txtImage_ = new Text(container, SWT.BORDER);
		txtImage_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblPortrait = new Label(container, SWT.NONE);
		lblPortrait.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblPortrait.setText("Profile:");

		txtProfile_ = new Text(container, SWT.BORDER);
		txtProfile_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblEllipse = new Label(container, SWT.NONE);
		lblEllipse.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblEllipse.setText("Ellipse:");

		txtEllipse_ = new Text(container, SWT.BORDER);
		txtEllipse_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblDieSound = new Label(container, SWT.NONE);
		lblDieSound.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblDieSound.setText("Die sound:");

		txtDieSound_ = new Text(container, SWT.BORDER);
		txtDieSound_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblUndeadVariation = new Label(container, SWT.NONE);
		lblUndeadVariation.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblUndeadVariation.setText("Undead variation:");

		txtUndeadVariation_ = new Text(container, SWT.BORDER);
		txtUndeadVariation_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 3, 1));

		Label lblZoc = new Label(container, SWT.NONE);
		lblZoc.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblZoc.setText("ZOC:");

		cmbZoc_ = new Combo(container, SWT.READ_ONLY);
		cmbZoc_.setItems(new String[] { "", "yes", "no" });
		cmbZoc_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		cmbZoc_.select(0);

		Label lblHideHelp = new Label(container, SWT.NONE);
		lblHideHelp.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblHideHelp.setText("Hide help:");

		cmbHideHelp_ = new Combo(container, SWT.READ_ONLY);
		cmbHideHelp_.setItems(new String[] { "false", "true" });
		cmbHideHelp_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		cmbHideHelp_.select(0);

		Label lblDescription = new Label(container, SWT.NONE);
		lblDescription.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblDescription.setText("Description:");

		txtDecscription_ = new Text(container, SWT.BORDER | SWT.MULTI);
		txtDecscription_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 7, 1));

		Label lblbaseunitId = new Label(container, SWT.NONE);
		lblbaseunitId.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblbaseunitId.setText("[base_unit] id:");

		txtBaseUnitId_ = new Text(container, SWT.BORDER);
		txtBaseUnitId_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		updatePageIsComplete();
	}

	private void updatePageIsComplete()
	{
		setPageComplete(false);
		setErrorMessage(null);
		if (getId().isEmpty())
		{
			setErrorMessage("ID cannot be empty");
			return;
		}
		setPageComplete(true);
	}

	public String getAlignment()
	{
		return cmbAlignment_.getText();
	}

	public String getId()
	{
		return txtId_.getText();
	}

	public String getUnitTypeName()
	{
		return txtUnitTypeName_.getText();
	}

	public String getAdvancesTo()
	{
		return txtAdvancesTo_.getText();
	}

	public String getCost()
	{
		return txtCost.getText();
	}

	public String getExperience()
	{
		return txtExperience.getText();
	}

	public String getHitpoints()
	{
		return txtHitpoints_.getText();
	}

	public String getLevel()
	{
		return txtLevel_.getText();
	}

	public String getMovement()
	{
		return txtMovement_.getText();
	}

	public String getMovementType()
	{
		return txtMovementType_.getText();
	}

	public String getRace()
	{
		return txtRace_.getText();
	}

	public String getTraitsNumber()
	{
		return txtTraitsNumber_.getText();
	}

	public String getUnitTypeImage()
	{
		return txtImage_.getText();
	}

	public String getProfile()
	{
		return txtProfile_.getText();
	}

	public String getUndeadVariation()
	{
		return txtUndeadVariation_.getText();
	}

	public String getZoc()
	{
		return cmbZoc_.getText();
	}

	public String getEllipse()
	{
		return txtEllipse_.getText();
	}

	public String getDieSound()
	{
		return txtDieSound_.getText();
	}

	public String getDecscription()
	{
		return txtDecscription_.getText();
	}

	public String getIgnoreTraits()
	{
		return cmbIgnoreTraits_.getText();
	}

	public String getHideHelp()
	{
		return cmbHideHelp_.getText();
	}

	public String getBaseUnitId()
	{
		return txtBaseUnitId_.getText();
	}

	public String getGender()
	{
		return cmbGender.getText();
	}

	public String getAttacks()
	{
		return txtAttacks.getText();
	}

	public String getUsage()
	{
		return txtUsage_.getText();
	}

}
/**
 * @author Timotei Dolean
 *
 */
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
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.jface.DoubleInputDialog;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ListUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class MoveTypeWizard extends NewWizardTemplate
{
	private MoveTypePage0	page0_;

	public MoveTypeWizard() {
		setWindowTitle("Movetype tag wizard");
	}

	@Override
	public void addPages()
	{
		page0_ = new MoveTypePage0();
		addPage(page0_);

		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		isFinished_ = true;
		ArrayList<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();

		objectName_ = page0_.getName();
		params.add(new ReplaceableParameter("$$movetype_name", page0_.getName()));
		params.add(new ReplaceableParameter("$$flies", String.valueOf(page0_.getFlies())));

		params.add(new ReplaceableParameter("$$movement_costs", page0_.getMovementCosts()));
		params.add(new ReplaceableParameter("$$defense", page0_.getDefense()));
		params.add(new ReplaceableParameter("$$resistance", page0_.getResistance()));

		String template = TemplateProvider.getInstance().getProcessedTemplate("movetype", params);

		if (template == null)
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Template for \"movetype\" not found.");
			return false;
		}
		data_ = template;
		return true;
	}

	public class MoveTypePage0 extends WizardPage
	{
		private Button	chkFlies;
		private Text	txtName_;
		private Group	grpmovementcosts;
		private Group	grpdefense;
		private List	lstMoveCosts_;
		private List	lstDef_;
		private Group	grpresistance;
		private List	lstResist_;
		private Button	btnAddRes;
		private Button	btnRemoveRes;

		public MoveTypePage0() {
			super("moveTypePage0");
			setTitle("Move type wizard");
			setDescription("Create a new [movetype]");
		}

		private void updatePageIsComplete()
		{
			setPageComplete(false);
			if (getName().isEmpty())
			{
				setErrorMessage("The movetype needs a unique name.");
				return;
			}
			setErrorMessage(null);
			setPageComplete(true);
		}

		@Override
		public void createControl(Composite parent)
		{
			parent.setLocation(0, 0);
			Composite container = new Composite(parent, SWT.NULL);

			setControl(container);
			container.setLayout(new GridLayout(3, false));

			Label lblName = new Label(container, SWT.NONE);
			GridData gd_lblName = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
			gd_lblName.widthHint = 68;
			lblName.setLayoutData(gd_lblName);
			lblName.setText("Name*:");

			txtName_ = new Text(container, SWT.BORDER);
			txtName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 2, 1));
			txtName_.addModifyListener(new ModifyListener() {
				@Override
				public void modifyText(ModifyEvent e)
				{
					updatePageIsComplete();
				}
			});

			chkFlies = new Button(container, SWT.CHECK);
			chkFlies.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 3, 1));
			chkFlies.setText("Flies");

			grpmovementcosts = new Group(container, SWT.NONE);
			grpmovementcosts.setText("[movement_costs]");
			grpmovementcosts.setLayout(null);
			GridData gd_grpmovementcosts = new GridData(SWT.FILL, SWT.CENTER, false, false, 2, 1);
			gd_grpmovementcosts.widthHint = 269;
			grpmovementcosts.setLayoutData(gd_grpmovementcosts);

			lstMoveCosts_ = new List(grpmovementcosts, SWT.BORDER);
			lstMoveCosts_.setBounds(4, 25, 190, 68);

			Button btnAddMove = new Button(grpmovementcosts, SWT.NONE);
			btnAddMove.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					addNewItem(lstMoveCosts_, "Terrain", "Speed");
				}
			});
			btnAddMove.setBounds(200, 24, 65, 25);
			btnAddMove.setText("Add");

			Button btnRemoveMove = new Button(grpmovementcosts, SWT.NONE);
			btnRemoveMove.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					removeSelectedItem(lstMoveCosts_);
				}
			});
			btnRemoveMove.setText("Remove");
			btnRemoveMove.setBounds(200, 68, 65, 25);

			grpdefense = new Group(container, SWT.NONE);
			grpdefense.setText("[defense]");
			grpdefense.setLayout(null);
			grpdefense.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));

			lstDef_ = new List(grpdefense, SWT.BORDER);
			lstDef_.setBounds(4, 25, 190, 68);

			Button btnRemoveDef = new Button(grpdefense, SWT.NONE);
			btnRemoveDef.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					removeSelectedItem(lstDef_);
				}
			});
			btnRemoveDef.setText("Remove");
			btnRemoveDef.setBounds(209, 68, 65, 25);

			Button btnAddDef = new Button(grpdefense, SWT.NONE);
			btnAddDef.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					addNewItem(lstDef_, "Terrain", "Defense");
				}
			});
			btnAddDef.setText("Add");
			btnAddDef.setBounds(209, 25, 65, 25);

			grpresistance = new Group(container, SWT.NONE);
			grpresistance.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 2, 1));
			grpresistance.setLayout(null);
			grpresistance.setText("[resistance]");

			lstResist_ = new List(grpresistance, SWT.BORDER);
			lstResist_.setBounds(4, 25, 190, 68);

			btnAddRes = new Button(grpresistance, SWT.NONE);
			btnAddRes.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					addNewItem(lstResist_, "Attack Type", "Resistance");
				}
			});
			btnAddRes.setText("Add");
			btnAddRes.setBounds(200, 24, 65, 25);

			btnRemoveRes = new Button(grpresistance, SWT.NONE);
			btnRemoveRes.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					removeSelectedItem(lstResist_);
				}
			});
			btnRemoveRes.setText("Remove");
			btnRemoveRes.setBounds(200, 68, 65, 25);
			new Label(container, SWT.NONE);
			updatePageIsComplete();
		}

		private void addNewItem(List targetList, String firstValueName, String secondValueName)
		{
			DoubleInputDialog dialog = new DoubleInputDialog(getShell(), firstValueName, secondValueName);
			if (dialog.open() == Window.OK)
				addItem(dialog.getFirstValue(), dialog.getSecondValue(), targetList);
		}

		private void addItem(String name, String value, List targetList)
		{
			if (targetList == null)
				return;
			targetList.add(name + "=" + value);
		}

		private void removeSelectedItem(List targetList)
		{
			if (targetList == null)
				return;
			if (targetList.getSelectionCount() == 0)
			{
				GUIUtils.showMessageBox("Please select an item before deleting it.");
				return;
			}
			targetList.remove(targetList.getSelectionIndex());
		}

		@Override
		public String getName()
		{
			return txtName_.getText();
		}

		public boolean getFlies()
		{
			return chkFlies.getSelection();
		}

		public String getDefense()
		{
			return ListUtils.concatenateArray(lstDef_.getItems(), "\n\t");
		}

		public String getResistance()
		{
			return ListUtils.concatenateArray(lstResist_.getItems(), "\n\t");
		}

		public String getMovementCosts()
		{
			return ListUtils.concatenateArray(lstMoveCosts_.getItems(), "\n\t");
		}
	}
}

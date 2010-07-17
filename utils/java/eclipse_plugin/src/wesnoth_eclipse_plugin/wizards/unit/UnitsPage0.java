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

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.TypedEvent;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ListUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;

@Deprecated
public class UnitsPage0 extends WizardPage
{
	private IStructuredSelection	selection_;
	private Text					txtDirectory_;
	private Text					txtFileName_;

	private List					selectedList_;

	java.util.List<String>			unitTypes_;
	java.util.List<String>			traits_;
	java.util.List<String>			moveTypes_;
	java.util.List<String>			races_;

	private List					lstUnitTypes_;
	private List					lstTraits_;
	private List					lstMoveTypes_;
	private List					lstRaces_;

	public UnitsPage0(IStructuredSelection selection) {
		super("unitPage0");
		setTitle("New unit wizard");
		setDescription("Create a new unit");

		selection_ = selection;

		unitTypes_ = new ArrayList<String>();
		traits_ = new ArrayList<String>();
		moveTypes_ = new ArrayList<String>();
		races_ = new ArrayList<String>();
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
		SelectionAdapter listSelection = new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				updateSelection(e);
			}
		};
		MouseAdapter listSelectionMouse = new MouseAdapter() {
			@Override
			public void mouseDown(MouseEvent e)
			{
				updateSelection(e);
			}
		};

		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		container.setLayout(null);

		Button button = new Button(container, SWT.NONE);
		button.setText("Browse...");
		button.setBounds(505, 10, 59, 25);
		button.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				handleBrowse();
			}
		});

		txtDirectory_ = new Text(container, SWT.BORDER);
		txtDirectory_.setBounds(90, 12, 409, 21);
		txtDirectory_.addModifyListener(modifyListener);

		Label label = new Label(container, SWT.NONE);
		label.setText("Directory* :");
		label.setBounds(10, 15, 75, 15);

		Label label_1 = new Label(container, SWT.NONE);
		label_1.setText("File name* :");
		label_1.setBounds(10, 43, 75, 15);

		txtFileName_ = new Text(container, SWT.BORDER);
		txtFileName_.setBounds(90, 40, 409, 21);
		txtFileName_.addModifyListener(modifyListener);

		lstUnitTypes_ = new List(container, SWT.BORDER);
		lstUnitTypes_.setBounds(10, 85, 120, 80);
		lstUnitTypes_.addSelectionListener(listSelection);
		lstUnitTypes_.addMouseListener(listSelectionMouse);
		lstUnitTypes_.setData(unitTypes_);

		Label lblUnitTypes = new Label(container, SWT.NONE);
		lblUnitTypes.setBounds(40, 64, 64, 15);
		lblUnitTypes.setText("Unit types:");

		lstTraits_ = new List(container, SWT.BORDER);
		lstTraits_.setBounds(155, 85, 120, 80);
		lstTraits_.addSelectionListener(listSelection);
		lstTraits_.addMouseListener(listSelectionMouse);
		lstTraits_.setData(traits_);

		Label lblMoveTypes = new Label(container, SWT.NONE);
		lblMoveTypes.setText("Move types:");
		lblMoveTypes.setBounds(326, 64, 75, 15);

		lstMoveTypes_ = new List(container, SWT.BORDER);
		lstMoveTypes_.setBounds(298, 85, 120, 80);
		lstMoveTypes_.addSelectionListener(listSelection);
		lstMoveTypes_.addMouseListener(listSelectionMouse);
		lstMoveTypes_.setData(moveTypes_);

		Button btnAdd = new Button(container, SWT.NONE);
		btnAdd.setBounds(191, 171, 84, 25);
		btnAdd.setText("Add");
		btnAdd.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				addNewItem();
			}
		});

		Button btnRemove = new Button(container, SWT.NONE);
		btnRemove.setBounds(298, 171, 84, 25);
		btnRemove.setText("Remove");
		btnRemove.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				removeSelectedItem();
			}
		});

		lstRaces_ = new List(container, SWT.BORDER);
		lstRaces_.setBounds(444, 85, 120, 80);
		lstRaces_.addSelectionListener(listSelection);
		lstRaces_.addMouseListener(listSelectionMouse);
		lstRaces_.setData(races_);

		Label lblRaces = new Label(container, SWT.NONE);
		lblRaces.setText("Races:");
		lblRaces.setBounds(478, 67, 46, 15);

		Label lblTraits = new Label(container, SWT.NONE);
		lblTraits.setText("Traits:");
		lblTraits.setBounds(180, 67, 64, 15);

		initialize();
		updatePageIsComplete();
	}

	private void updateSelection(TypedEvent e)
	{
		if (!(e.getSource() instanceof List))
			return;

		if (selectedList_ != null && selectedList_ != (List) e.getSource())
			selectedList_.deselectAll();
		selectedList_ = (List) e.getSource();
	}

	@SuppressWarnings("unchecked")
	private void removeSelectedItem()
	{
		if (selectedList_ == null || selectedList_.getSelection().length == 0)
		{
			GUIUtils.showWarnMessageBox("Please select an item from a list first.");
			return;
		}
		if (selectedList_.getData() != null && selectedList_.getData() instanceof java.util.List<?>)
			((java.util.List<String>) selectedList_.getData()).remove(selectedList_.getSelectionIndex());
		selectedList_.remove(selectedList_.getSelectionIndex());
	}

	@SuppressWarnings("unchecked")
	private void addNewItem()
	{
		if (selectedList_ == null)
		{
			GUIUtils.showWarnMessageBox("Please select a list first.");
			return;
		}

		NewWizardTemplate wizard = null;
		if (selectedList_.hashCode() == lstMoveTypes_.hashCode())
		{
			wizard = new MoveTypeWizard();
		}
		else if (selectedList_.hashCode() == lstRaces_.hashCode())
		{
			wizard = new RaceWizard();
		}
		else if (selectedList_.hashCode() == lstTraits_.hashCode())
		{
			//wizard = new TraitWizard();
			GUIUtils.showWarnMessageBox("Not implemented yet");
			return;
		}
		else if (selectedList_.hashCode() == lstUnitTypes_.hashCode())
		{
			wizard = new UnitTypeWizard();
		}

		if (wizard == null)
			return;
		wizard.init(Activator.getDefault().getWorkbench(), selection_);
		wizard.setForcePreviousAndNextButtons(true);

		WizardDialog wizardDialog = new WizardDialog(getShell(), wizard);
		wizardDialog.create();
		wizardDialog.getShell().setLocation(getShell().getBounds().x, getShell().getBounds().y);
		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(wizardDialog.getShell(),
				"org.eclipse.ui.new_wizard_context");

		wizardDialog.open();

		if (!wizard.isFinished())
			return;

		if (selectedList_.getData() != null && selectedList_.getData() instanceof java.util.List<?>)
			((java.util.List<String>) selectedList_.getData()).add(wizard.getData().toString());
		selectedList_.add(wizard.getObjectName());
	}

	private void updatePageIsComplete()
	{
		IResource container = ResourcesPlugin.getWorkspace().getRoot().findMember(new Path(getDirectoryName()));
		setPageComplete(false);
		String fileName = getFileName();

		if (getDirectoryName().isEmpty())
		{
			setErrorMessage("You need to specify a valid directory path first.");
			return;
		}

		if (container == null || !container.exists() || !(container instanceof IContainer))
		{
			setErrorMessage("The directory must be created first and the selected folder to exist.");
			return;
		}

		if (fileName.isEmpty())
		{
			setErrorMessage("File name must be specified.");
			return;
		}

		if (fileName.replace('\\', '/').indexOf('/', 1) > 0)
		{
			setErrorMessage("File name must be valid.");
			return;
		}

		int dotLoc = fileName.lastIndexOf('.');
		if (dotLoc == -1 || fileName.substring(dotLoc + 1).equalsIgnoreCase("cfg") == false)
		{
			setErrorMessage("File extension must be 'cfg'.");
			return;
		}

		setErrorMessage(null);
		setPageComplete(true);
	}

	/**
	 * Tests if the current workbench selection is a suitable directory to use.
	 */
	private void initialize()
	{
		if (selection_ != null && selection_.isEmpty() == false &&
				selection_ instanceof IStructuredSelection)
		{
			IStructuredSelection ssel = selection_;
			if (ssel.size() > 1)
			{
				return;
			}
			Object obj = ssel.getFirstElement();
			if (obj instanceof IResource)
			{
				IContainer container;
				if (obj instanceof IContainer)
				{
					container = (IContainer) obj;
				}
				else
				{
					container = ((IResource) obj).getParent();
				}
				txtDirectory_.setText(container.getFullPath().toString());
			}
		}
	}

	private void handleBrowse()
	{
		ContainerSelectionDialog dialog =
				new ContainerSelectionDialog(getShell(), ResourcesPlugin.getWorkspace().getRoot(), false,
						"Select a campaign project");
		if (dialog.open() == ContainerSelectionDialog.OK)
		{
			Object[] result = dialog.getResult();
			if (result.length == 1)
			{
				txtDirectory_.setText(((Path) result[0]).toString());
			}
		}
	}

	public String getDirectoryName()
	{
		return txtDirectory_.getText();
	}

	public String getFileName()
	{
		return txtFileName_.getText();
	}

	public String getUnitTypes()
	{
		return ListUtils.concatenateList(unitTypes_, "\n\t");
	}

	public String getTraits()
	{
		return ListUtils.concatenateList(traits_, "\n\t");
	}

	public String getMoveTypes()
	{
		return ListUtils.concatenateList(moveTypes_, "\n\t");
	}

	public String getRaces()
	{
		return ListUtils.concatenateList(races_, "\n\t");
	}
}

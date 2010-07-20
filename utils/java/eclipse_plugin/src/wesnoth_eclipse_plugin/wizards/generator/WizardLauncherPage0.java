/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.generator;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.utils.EditorUtils;
import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class WizardLauncherPage0 extends NewWizardPageTemplate
{
	private Text					txtDirectory_;
	private Text					txtFileName_;
	private Button					radioNewFile;
	private Label					lblCurrentFileOpened;
	private Label					lblDirectory;
	private Button					btnBrowse;
	private Label					lblFileName;

	public WizardLauncherPage0() {
		super("wizardLauncherPage0");
		setTitle("Wizard launcher");
		setDescription("Select destination");
	}

	@Override
	public void createControl(Composite parent)
	{
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		container.setLayout(new GridLayout(4, false));

		radioNewFile = new Button(container, SWT.RADIO);
		radioNewFile.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				updateEnabledStatus();
			}
		});
		radioNewFile.setSelection(true);
		radioNewFile.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 2, 1));
		radioNewFile.setText("New file");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		lblDirectory = new Label(container, SWT.NONE);
		lblDirectory.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblDirectory.setText("Directory* :");

		txtDirectory_ = new Text(container, SWT.BORDER);
		txtDirectory_.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e)
			{
				updatePageIsComplete();
			}
		});
		txtDirectory_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		txtDirectory_.setEditable(false);

		btnBrowse = new Button(container, SWT.NONE);
		btnBrowse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				Path path = handleBrowseContainer();
				if (path != null)
					txtDirectory_.setText(path.toString());
			}
		});

		btnBrowse.setText("Browse...");
		new Label(container, SWT.NONE);

		lblFileName = new Label(container, SWT.NONE);
		lblFileName.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblFileName.setText("File name* :");

		txtFileName_ = new Text(container, SWT.BORDER);
		txtFileName_.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e)
			{
				updatePageIsComplete();
			}
		});
		txtFileName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		Button radioCurrentFile = new Button(container, SWT.RADIO);
		radioCurrentFile.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				updateEnabledStatus();
			}
		});
		radioCurrentFile.setLayoutData(new GridData(SWT.LEFT, SWT.CENTER, false, false, 2, 1));
		radioCurrentFile.setText("In current edited file");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		lblCurrentFileOpened = new Label(container, SWT.NONE);
		lblCurrentFileOpened.setEnabled(false);
		lblCurrentFileOpened.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 2, 1));
		lblCurrentFileOpened.setText("Current file opened: ");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		Label label = new Label(container, SWT.NONE);
		label.setText("            ");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		txtDirectory_.setText(getWizard().getSelectionContainer().getFullPath().toString());
		updatePageIsComplete();
	}

	private void updatePageIsComplete()
	{
		setPageComplete(false);
		if (radioNewFile.getSelection())
		{
			IResource container = ResourcesPlugin.getWorkspace().getRoot().findMember(new Path(getDirectoryName()));
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
		}
		else
		{
			// current file checking
			if (EditorUtils.getEditedFile() != null)
			{
				lblCurrentFileOpened.setText("File " + EditorUtils.getEditedFile().getEditorInput().getName() + " opened.");
			}
			else
			{
				lblCurrentFileOpened.setText("No file opened.");
				setErrorMessage("No file opened.");
				return;
			}
		}
		setPageComplete(true);
		setErrorMessage(null);
	}

	public void updateEnabledStatus()
	{
		// new file section
		btnBrowse.setEnabled(radioNewFile.getSelection());
		lblDirectory.setEnabled(radioNewFile.getSelection());
		lblFileName.setEnabled(radioNewFile.getSelection());
		txtDirectory_.setEnabled(radioNewFile.getSelection());
		txtFileName_.setEnabled(radioNewFile.getSelection());

		// opened file
		lblCurrentFileOpened.setEnabled(!radioNewFile.getSelection());

		txtDirectory_.setText(getWizard().getSelectionContainer().getFullPath().toString());
		updatePageIsComplete();
	}

	public String getFileName()
	{
		return radioNewFile.getSelection() == true ? txtFileName_.getText() :
				EditorUtils.getEditedFile().getEditorInput().getName();
	}

	public String getDirectoryName()
	{
		return radioNewFile.getSelection() == true ? txtDirectory_.getText() : "";
	}

	public boolean getIsTargetNewFile()
	{
		return radioNewFile.getSelection();
	}
}

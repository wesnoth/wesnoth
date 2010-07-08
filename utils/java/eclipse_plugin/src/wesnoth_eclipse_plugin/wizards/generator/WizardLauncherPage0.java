/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.IStructuredSelection;
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
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import wesnoth_eclipse_plugin.Activator;

public class WizardLauncherPage0 extends WizardPage
{
	private IStructuredSelection	selection_;

	private Text					txtDirectory_;
	private Text					txtFileName_;
	private Button					radioNewFile;
	private Label					lblCurrentFileOpened;
	private Label					lblDirectory;
	private Button					btnBrowse;
	private Label					lblFileName;

	public WizardLauncherPage0(IStructuredSelection selection) {
		super("wizardLauncherPage0");
		setTitle("Wizard launcher");
		setDescription("Select destination");
		selection_ = selection;
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

		btnBrowse = new Button(container, SWT.NONE);
		btnBrowse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				handleBrowse();
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
		initialize();
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
			if (getEditedFile() != null)
			{
				lblCurrentFileOpened.setText("File " + getEditedFile().getEditorInput().getName() + " opened.");
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

	public IEditorPart getEditedFile()
	{
		return Activator.getDefault().getWorkbench().getActiveWorkbenchWindow().getPages()[0].getActiveEditor();
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

		updatePageIsComplete();
	}

	/**
	 * Tests if the current workbench selection is a suitable directory to use.
	 */
	private void initialize()
	{
		if (selection_ != null && selection_.isEmpty() == false &&
				selection_ instanceof IStructuredSelection && selection_.size() > 0)
		{
			Object obj = selection_.getFirstElement();
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

	/**
	 * Uses the standard container selection dialog to choose the new value for
	 * the the directory field.
	 */
	private void handleBrowse()
	{
		ContainerSelectionDialog dialog =
				new ContainerSelectionDialog(getShell(), ResourcesPlugin.getWorkspace().getRoot(), false,
						"Select a directory");
		if (dialog.open() == ContainerSelectionDialog.OK)
		{
			Object[] result = dialog.getResult();
			if (result.length == 1)
			{
				txtDirectory_.setText(((Path) result[0]).toString());
			}
		}
	}

	public String getFileName()
	{
		return radioNewFile.getSelection() == true ? txtFileName_.getText() : getEditedFile().getEditorInput().getName();
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

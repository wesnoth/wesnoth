/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.jface;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

public class DoubleInputDialog extends Dialog
{
	private Text	txtValue1_;
	private Text	txtValue2_;
	private Label	lblValue1;
	private Label	lblValue2;

	private String	resStr1	= "", resStr2 = "";
	private String	val1String	= "", val2String = "";

	public DoubleInputDialog(Shell parentShell, String value1String, String value2String) {
		super(parentShell);
		setShellStyle(SWT.DIALOG_TRIM);
		val1String = value1String + ":";
		val2String = value2String + ":";
	}

	@Override
	protected Control createDialogArea(Composite parent)
	{
		Composite container = (Composite) super.createDialogArea(parent);
		GridLayout gridLayout = (GridLayout) container.getLayout();
		gridLayout.numColumns = 3;

		lblValue1 = new Label(container, SWT.NONE);
		lblValue1.setText(val1String);
		Label label = new Label(container, SWT.NONE);
		label.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));

		txtValue1_ = new Text(container, SWT.BORDER);
		txtValue1_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		lblValue2 = new Label(container, SWT.NONE);
		lblValue2.setText(val2String);
		Label label_3 = new Label(container, SWT.NONE);
		label_3.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));

		txtValue2_ = new Text(container, SWT.BORDER);
		txtValue2_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

		return container;
	}

	@Override
	protected void createButtonsForButtonBar(Composite parent)
	{
		createButton(parent, IDialogConstants.OK_ID, IDialogConstants.OK_LABEL, true);
		createButton(parent, IDialogConstants.CANCEL_ID, IDialogConstants.CANCEL_LABEL, false);
	}

	@Override
	public boolean close()
	{
		resStr1 = txtValue1_.getText();
		resStr2 = txtValue2_.getText();
		return super.close();
	}

	@Override
	protected Point getInitialSize()
	{
		return new Point(385, 155);
	}

	public String getFirstValue()
	{
		return resStr1;
	}

	public String getSecondValue()
	{
		return resStr2;
	}
}

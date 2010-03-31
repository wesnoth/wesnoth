/**
 * 
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.Composite;
import org.wesnoth.wml.schema.SchemaTag;

/**
 * @author fabi
 *
 */
public class WmlTagWizardPage extends WizardPage {

	/**
	 * @param pageName
	 */
	public WmlTagWizardPage(SchemaTag tag, ImageDescriptor titleImage) {
		super(tag.getName(), tag.getName(), titleImage);
	}

//	/**
//	 * @param pageName
//	 * @param title
//	 * @param titleImage
//	 */
//	public WmlTagWizardPage(String pageName, String title,
//			ImageDescriptor titleImage) {
//		super(pageName, title, titleImage);
//		// TODO Auto-generated constructor stub
//	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.dialogs.IDialogPage#createControl(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	public void createControl(Composite parent) {
		// TODO Auto-generated method stub

	}

}

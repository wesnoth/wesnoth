/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.xtext.ui.editor.XtextEditor;

public class WMLEditor extends XtextEditor
{
	@Override
	public void createPartControl(Composite parent)
	{
		super.createPartControl(parent);
		//		listener_ = new WMLSelectionChangeListener(DefaultUiModule.class.getTypeParameters());
		//		listener_.install(getSelectionProvider());
	}

	@Override
	public void dispose()
	{
		//listener_.uninstall(getSelectionProvider());
		super.dispose();
	}
}

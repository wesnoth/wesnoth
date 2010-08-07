/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.editor;

import org.eclipse.xtext.ui.editor.AbstractDirtyStateAwareEditorCallback;
import org.eclipse.xtext.ui.editor.XtextEditor;

public class WMLAbstractDirtyEditorCallback extends AbstractDirtyStateAwareEditorCallback
{
	@Override
	public void afterCreatePartControl(XtextEditor editor)
	{
		super.afterCreatePartControl(editor);
	}
}

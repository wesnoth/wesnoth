/**
 * Copyright (c) 2006-2009, Cloudsmith Inc.
 * The code, documentation and other materials contained herein have been
 * licensed under the Eclipse Public License - v 1.0 by the copyright holder
 * listed above, as the Initial Contributor under such license. The text of
 * such license is available at www.eclipse.org.
 */

package org.wesnoth.ui.editor;

import org.eclipse.jface.action.IMenuManager;

import com.google.inject.ImplementedBy;

/**
 * 
 *
 */
@ImplementedBy(DefaultExtXtextEditorCustomizer.class)
public interface IExtXtextEditorCustomizer {
	public void customizeEditorContextMenu(IMenuManager menuManager);
}

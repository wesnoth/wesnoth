/**
 * Copyright (c) 2010, Cloudsmith Inc.
 * The code, documentation and other materials contained herein have been
 * licensed under the Eclipse Public License - v 1.0 by the copyright holder
 * listed above, as the Initial Contributor under such license. The text of
 * such license is available at www.eclipse.org.
 */

package org.wesnoth.ui.xtext;

import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.IURIEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.xtext.ui.editor.XtextEditor;

/**
 * This class extends the standard XtextEditor to make it capable of
 * opening and saving external files and opening any URI (supported by URL scheme)
 * in read only mode.
 */
public class EFSExtendedXtextEditor extends XtextEditor {
	/**
	 * Does nothing except server as a place to set a breakpoint :)
	 */
	public EFSExtendedXtextEditor() {
		super();
	}

	/**
	 * Translates an incoming IEditorInput being an FilestoreEditorInput, or IURIEditorInput
	 * that is not also a IFileEditorInput.
	 * FilestoreEditorInput is used when opening external files in an IDE environment.
	 * The result is that the regular XtextEditor gets an IEFSEditorInput which is also an
	 * IStorageEditorInput.
	 */
	@Override
	public void init(IEditorSite site, IEditorInput input) throws PartInitException {
		// THE ISSUE HERE:
		// In the IDE, the File Open Dialog (and elsewhere) uses a FilestoreEditorInput class
		// which is an IDE specific implementation.
		// The state at this point:
		// 1. When creating a file, the IEditorInput is an IURIEditorInput
		// 2. The only (non IDE specific) interface implemented by FilestoreEditorInput is IURIEditorInput
		// 3. The creation of a file is however also an IFileEditorInput
		//
		// Remedy:
		// 1. Create an IEditorInput that can be used by the rest of Xtext
		// 2. If it is a file based URI select an impl that can handle both reading, writing and annotations
		// even if not in the workbench
		// 3. If it is a non file: uri, provide an impl that at least opens the file
		// 3.1 The EMF URIEditorInputImpl does not support writing, and does not automatically provide
		// markers.
		// 4. The XtextDocumentProvider must be specialized to handle the specialized IEditorInput
		// if it can not be made to look like one of the others (too much may be assumed)
		//
		if(input instanceof IURIEditorInput && !(input instanceof IFileEditorInput)) {
			super.init(site, new EFSEditorInput(((IURIEditorInput) input).getURI(), input.getName()));
			return;
		}
		super.init(site, input);
	}
}

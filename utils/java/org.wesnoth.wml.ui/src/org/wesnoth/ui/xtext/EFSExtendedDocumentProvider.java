/**
 * Copyright (c) 2006-2009, Cloudsmith Inc.
 * The code, documentation and other materials contained herein have been
 * licensed under the Eclipse Public License - v 1.0 by the copyright holder
 * listed above, as the Initial Contributor under such license. The text of
 * such license is available at www.eclipse.org.
 */

package org.wesnoth.ui.xtext;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.ui.IEditorInput;
import org.eclipse.xtext.ui.editor.model.XtextDocumentProvider;

/**
 * An extension to XtextDocumentProvider to handle missing parts when dealing with
 * (workbench) external files.
 * 
 */
public class EFSExtendedDocumentProvider extends XtextDocumentProvider {
	/**
	 * Override to see if it gets instantiated.
	 */
	public EFSExtendedDocumentProvider() {
		super();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.editors.text.StorageDocumentProvider#createDocument(java.lang.Object)
	 */
	@Override
	protected IDocument createDocument(Object element) throws CoreException {
		// if(element instanceof FileStoreEditorInput) {
		// TextFileDocumentProvider fileProvider = new TextFileDocumentProvider(this);
		// fileProvider.IDocument doc = fileProvider.getDocument(element);
		// return doc;
		// }
		return super.createDocument(element);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.editors.text.StorageDocumentProvider#getDefaultEncoding()
	 */
	@Override
	public String getDefaultEncoding() {
		String encoding = super.getDefaultEncoding();
		System.err.printf("Encoding is: %s\n", encoding);
		if(encoding == null)
			return "UTF-8";
		return encoding;
	}

	/*
	 * @see IStorageDocumentProvider#getEncoding(Object)
	 * 
	 * @since 2.0
	 */
	@Override
	public String getEncoding(Object element) {
		if(element instanceof IEFSEditorInput) {
			// Don't think encoding is all that simple to get for EFS
			// Assuming it is used for "file system files" and should use
			// the default for the platform...
			String encoding = getDefaultEncoding();
			if(encoding == null)
				return "UTF-8";
			return encoding;
		}
		return super.getEncoding(element);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.xtext.ui.editor.model.XtextDocumentProvider#setDocumentContent(org.eclipse.jface.text.IDocument, org.eclipse.ui.IEditorInput,
	 * java.lang.String)
	 */
	@Override
	protected boolean setDocumentContent(IDocument document, IEditorInput editorInput, String encoding)
			throws CoreException {
		// if(editorInput instanceof URIEditorInput) {
		// URL inputURL;
		// InputStream stream = null;
		// try {
		// inputURL = new URL(((URIEditorInput) editorInput).getURI().toString());
		// stream = inputURL.openStream();
		// }
		// catch(IOException e) {
		// throw new CoreException(new Status(
		// Status.ERROR, BeeLangActivator.getInstance().getBundle().getSymbolicName(), e.getMessage(), e));
		// }
		// try {
		// // TODO: get encoding from some other place - if it is http, etc. it will need to be handled much differently
		// setDocumentContent(document, stream, "UTF-8");
		// }
		// finally {
		// try {
		// stream.close();
		// }
		// catch(IOException x) {
		// /* gulp */
		// }
		// }
		// return true;
		// }
		return super.setDocumentContent(document, editorInput, encoding);
	}
}

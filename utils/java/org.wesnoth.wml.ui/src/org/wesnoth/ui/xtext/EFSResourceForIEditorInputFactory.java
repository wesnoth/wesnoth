/**
 * Copyright (c) 2010, Cloudsmith Inc.
 * The code, documentation and other materials contained herein have been
 * licensed under the Eclipse Public License - v 1.0 by the copyright holder
 * listed above, as the Initial Contributor under such license. The text of
 * such license is available at www.eclipse.org.
 */

package org.wesnoth.ui.xtext;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.emf.common.util.URI;
import org.eclipse.emf.common.util.WrappedException;
import org.eclipse.emf.ecore.resource.Resource;
import org.eclipse.emf.ecore.resource.ResourceSet;
import org.eclipse.ui.IEditorInput;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.model.JavaClassPathResourceForIEditorInputFactory;

/**
 * Factory that handles regular files, outside of the Eclipse Workspace.
 * 
 */
public class EFSResourceForIEditorInputFactory extends JavaClassPathResourceForIEditorInputFactory {

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.xtext.ui.editor.model.ResourceForIEditorInputFactory#createResource(org.eclipse.ui.IEditorInput)
	 */
	@Override
	public Resource createResource(IEditorInput editorInput) {

		if(editorInput instanceof IEFSEditorInput)
			try {
				return createResourceFor((IEFSEditorInput) editorInput);
			}
			catch(CoreException e) {
				throw new WrappedException(e);
			}
		return super.createResource(editorInput);
	}

	protected Resource createResourceFor(IEFSEditorInput efsInput) throws CoreException {
		ResourceSet resourceSet = getResourceSet(efsInput.getStorage());
		URI uri = URI.createURI(efsInput.getURI().toString(), true);
		configureResourceSet(resourceSet, uri);
		XtextResource resource = createResource(resourceSet, uri);
		resource.setValidationDisabled(false);
		return resource;
	}
}

/**
 * Copyright (c) 2010, Cloudsmith Inc.
 * The code, documentation and other materials contained herein have been
 * licensed under the Eclipse Public License - v 1.0 by the copyright holder
 * listed above, as the Initial Contributor under such license. The text of
 * such license is available at www.eclipse.org.
 */

package org.wesnoth.ui.xtext;

import java.net.URI;
import java.net.URISyntaxException;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.IElementFactory;
import org.eclipse.ui.IMemento;

/**
 * Factory for saving and restoring a <code>EFSEditorInput</code>.
 * The stored representation of a <code>EFSEditorInput</code> remembers
 * the full URI of the file (that is, <code>IFilestore.getURI()</code>).
 * <p>
 * The workbench will automatically create instances of this class as required. It is not intended to be instantiated or subclassed by the client.
 * </p>
 * 
 * @noinstantiate This class is not intended to be instantiated by clients.
 * @noextend This class is not intended to be subclassed by clients.
 */
public class EFSEditorInputFactory implements IElementFactory {
	/**
	 * Factory id. The workbench plug-in registers a factory by this name
	 * with the "org.eclipse.ui.elementFactories" extension point.
	 */
	private static final String ID_FACTORY = "org.eclipse.ui.part.EFSEditorInputFactory"; //$NON-NLS-1$

	/**
	 * Tag for the IFilestore URI of the file resource.
	 */
	private static final String TAG_URI = "uri"; //$NON-NLS-1$

	private static final String TAG_NAME = "name"; //$NON-NLS-1$

	/**
	 * Returns the element factory id for this class.
	 * 
	 * @return the element factory id
	 */
	public static String getFactoryId() {
		return ID_FACTORY;
	}

	/**
	 * Saves the state of the given file editor input into the given memento.
	 * 
	 * @param memento
	 *            the storage area for element state
	 * @param input
	 *            the file editor input
	 */
	public static void saveState(IMemento memento, EFSEditorInput input) {
		memento.putString(TAG_URI, input.getURI().toString());
		memento.putString(TAG_NAME, input.getName().toString());
	}

	/**
	 * Creates a new factory.
	 */
	public EFSEditorInputFactory() {
	}

	/*
	 * (non-Javadoc)
	 * Method declared on IElementFactory.
	 */
	public IAdaptable createElement(IMemento memento) {
		// Get the file name.
		String fileName = memento.getString(TAG_NAME);
		String uriString = memento.getString(TAG_URI);
		if(fileName == null) {
			return null;
		}
		if(uriString == null) {
			return null;
		}

		// Get a handle to the IFile...which can be a handle
		// to a resource that does not exist in workspace
		IFile file = ResourcesPlugin.getWorkspace().getRoot().getFile(new Path(fileName));
		if(file != null) {
			try {
				return new EFSEditorInput(new URI(uriString), fileName);
			}
			catch(URISyntaxException e) {
				return null; // sorry, the URI was malformed, should never have gotten this far...
			}
		}
		return null;
	}
}

/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.builder;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IResourceDelta;
import org.eclipse.core.resources.IResourceDeltaVisitor;
import org.eclipse.core.resources.IResourceVisitor;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.globalactions.PreprocessorActions;
import wesnoth_eclipse_plugin.preferences.Preferences;
import wesnoth_eclipse_plugin.utils.AntUtils;
import wesnoth_eclipse_plugin.utils.Pair;
import wesnoth_eclipse_plugin.utils.ResourceUtils;
import wesnoth_eclipse_plugin.utils.StringUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

//TODO: rewrite and use a ".wesnoth" file to store additional infos instead of just ".ignore"
public class WesnothProjectBuilder extends IncrementalProjectBuilder
{
	public static final String BUILDER_ID = "Wesnoth_Eclipse_Plugin.projectBuilder";
	private static final String MARKER_TYPE = "Wesnoth_Eclipse_Plugin.configProblem";

	/**
	 * The key is the project name
	 * The value is:
	 * - the last modified date for the .ignore file
	 * - the list with ignored directories names
	 */
	private static HashMap<String, Pair<Long, List<String>>> ignoreCache_;

	public WesnothProjectBuilder() {
		if (ignoreCache_ == null)
			ignoreCache_ = new HashMap<String, Pair<Long, List<String>>>();
	}

	class SampleDeltaVisitor implements IResourceDeltaVisitor
	{
		private IProgressMonitor monitor_;

		public SampleDeltaVisitor(IProgressMonitor monitor) {
			monitor_ = monitor;
		}

		@Override
		public boolean visit(IResourceDelta delta) throws CoreException
		{
			IResource resource = delta.getResource();
			switch (delta.getKind())
			{
			case IResourceDelta.ADDED:
				// handle added resource
				checkResource(resource, monitor_);
				break;
			case IResourceDelta.REMOVED:
				// handle removed resource
				break;
			case IResourceDelta.CHANGED:
				// handle changed resource
				checkResource(resource, monitor_);
				break;
			}
			// return true to continue visiting children.
			return true;
		}
	}

	class SampleResourceVisitor implements IResourceVisitor
	{
		private IProgressMonitor monitor_;

		public SampleResourceVisitor(IProgressMonitor monitor) {
			monitor_ = monitor;
		}

		@Override
		public boolean visit(IResource resource)
		{
			checkResource(resource, monitor_);
			// return true to continue visiting children.
			return true;
		}
	}

	protected void fullBuild(final IProgressMonitor monitor) throws CoreException
	{
		try
		{
			getProject().accept(new SampleResourceVisitor(monitor));
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}

	protected void incrementalBuild(IResourceDelta delta, IProgressMonitor monitor) throws CoreException
	{
		// the visitor does the work.
		delta.accept(new SampleDeltaVisitor(monitor));
	}

	@SuppressWarnings("rawtypes")
	@Override
	protected IProject[] build(int kind, Map args, IProgressMonitor monitor) throws CoreException
	{
		Logger.getInstance().log("building...");
		monitor.beginTask("Building...", 100);

		monitor.subTask("Checking conditions...");
		if (Preferences.getString(Constants.P_WESNOTH_USER_DIR).isEmpty())
		{
			Logger.getInstance().log("no preferences set (project builder)",
					"Please set the wesnoth user dir before creating the content");
			return null;
		}

		// run the ant job to copy the whole project
		// in the user add-ons directory (incremental)
		if (!(new File(getProject().getLocation().toOSString() + "/build.xml").exists()))
		{
			Logger.getInstance().log("build.xml is missing",
					"The 'build.xml' file is missing. The building cannot continue.");
			// TODO: better way of handling this - maybe regenerating?
			return null;
		}
		monitor.worked(2);

		// get the directories list to ignore
		File ignoreFile = new File(getProject().getLocation().toOSString() + Path.SEPARATOR + ".ignore");
		if (!ignoreCache_.containsKey(getProject().getName()) || ignoreFile.lastModified() != ignoreCache_.get(getProject().getName()).First)
		{
			String contents = ResourceUtils.getFileContents(ignoreFile);
			if (contents != null)
			{
				List<String> list = new ArrayList<String>();
				String[] lines = StringUtils.getLines(contents);
				for (String line : lines)
					list.add(line);

				ignoreCache_.remove(getProject().getName());
				ignoreCache_.put(getProject().getName(), new Pair<Long, List<String>>(ignoreFile.lastModified(), list));
			}
		}
		monitor.worked(5);

		// Ant copy
		monitor.subTask("Copying resources...");
		HashMap<String, String> properties = new HashMap<String, String>();
		properties.put("wesnoth.user.dir", Preferences.getString(Constants.P_WESNOTH_USER_DIR) + Path.SEPARATOR);
		Logger.getInstance().log("Ant result:");
		String result = AntUtils.runAnt(getProject().getLocation().toOSString() + "/build.xml",
				properties, true);
		Logger.getInstance().log(result);

		monitor.worked(10);

		if (result == null)
		{
			Logger.getInstance().log("error running the ant job",
					"There was an error running the ant job.");
			return null;
		}

		// create the temporary directory used by the plugin if not created
		monitor.subTask("Creating temporary directory...");
		WorkspaceUtils.getTemporaryFolder();
		monitor.worked(2);

		if (kind == FULL_BUILD)
		{
			fullBuild(monitor);
		}
		else
		{
			IResourceDelta delta = getDelta(getProject());
			if (delta == null)
			{
				fullBuild(monitor);
			}
			else
			{
				incrementalBuild(delta, monitor);
			}
		}

		monitor.done();
		return null;
	}

	void checkResource(IResource resource, IProgressMonitor monitor)
	{
		monitor.worked(5);
		if (isResourceIgnored(resource))
			return;

		// config files
		if (resource instanceof IFile && (resource.getName().toLowerCase(Locale.ENGLISH).endsWith(".cfg")))
		{
			try
			{
				IFile file = (IFile) resource;
				deleteMarkers(file);

				monitor.subTask("Preprocessing...");
				PreprocessorActions.preprocessFile(WorkspaceUtils.getPathRelativeToUserDir(file), WorkspaceUtils.getTemporaryFolder(), null, true, false);
				monitor.worked(5);

				// TODO: here be dragons
				// - add markers for wmllint, wmlscope
				// - need a better output from wmltools

				/*
				IMarker[] resIMarkers = file.findMarkers(MARKER_TYPE, false, IResource.DEPTH_ZERO);
				Logger.print("found markers: " + resIMarkers.length);

				ExternalToolInvoker invoker = new ExternalToolInvoker(TOOL_PATH, resource.getFullPath().toOSString(), true);
				Logger.print("Tool: "+TOOL_PATH+ " checking file: "+resource.getFullPath().toOSString());
				invoker.run();
				invoker.waitFor();

				// we need to find the correct column start/end based on the current document
				// (or get that from the tool)
				IDocumentProvider provider = new TextFileDocumentProvider();
				provider.connect(file);
				IDocument document = provider.getDocument(file);

				String line;MarkerToken token;
				while((line  = invoker.readOutputLine()) != null)
				{
					token = MarkerToken.parseToken(line);
					IMarker marker = file.createMarker(MARKER_TYPE);
					marker.setAttribute(IMarker.MESSAGE, token.getMessage());
					if (token.getColumnEnd() != 0)
					{
						marker.setAttribute(IMarker.CHAR_START,document.getLineOffset(token.getLine()-1) + token.getColumnStart());
						marker.setAttribute(IMarker.CHAR_END, document.getLineOffset(token.getLine()-1) + token.getColumnEnd());
					}
					marker.setAttribute(IMarker.LINE_NUMBER, token.getLine());
					marker.setAttribute(IMarker.SEVERITY,token.getType().toMarkerSeverity());
				}
				*/

			} catch (Exception e)
			{
				Logger.getInstance().logException(e);
			}
		}
	}

	private boolean isResourceIgnored(IResource res)
	{
		// we have an ignore cache
		if (ignoreCache_.containsKey(getProject().getName()))
		{
			List<String> ignoreList = ignoreCache_.get(getProject().getName()).Second;
			for (String path : ignoreList)
			{
				if (StringUtils.normalizePath(WorkspaceUtils.getPathRelativeToUserDir(res))
						.contains(StringUtils.normalizePath(path)))
					return true;
			}
		}
		return false;
	}

	private void addMarker(IFile file, String message, int lineNumber, int severity)
	{
		try
		{
			IMarker marker = file.createMarker(MARKER_TYPE);
			marker.setAttribute(IMarker.MESSAGE, message);
			marker.setAttribute(IMarker.SEVERITY, severity);
			if (lineNumber == -1)
			{
				lineNumber = 1;
			}
			marker.setAttribute(IMarker.LINE_NUMBER, lineNumber);
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}

	private void deleteMarkers(IFile file)
	{
		try
		{
			file.deleteMarkers(MARKER_TYPE, false, IResource.DEPTH_ZERO);
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}
}

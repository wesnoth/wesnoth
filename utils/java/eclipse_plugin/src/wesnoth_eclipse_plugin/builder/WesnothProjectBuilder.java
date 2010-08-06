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

import org.eclipse.core.resources.IContainer;
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
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.text.IDocument;
import org.eclipse.ui.editors.text.TextFileDocumentProvider;
import org.eclipse.ui.texteditor.IDocumentProvider;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;
import wesnoth_eclipse_plugin.templates.ReplaceableParameter;
import wesnoth_eclipse_plugin.utils.AntUtils;
import wesnoth_eclipse_plugin.utils.PreprocessorUtils;
import wesnoth_eclipse_plugin.utils.ProjectCache;
import wesnoth_eclipse_plugin.utils.ProjectUtils;
import wesnoth_eclipse_plugin.utils.ResourceUtils;
import wesnoth_eclipse_plugin.utils.StringUtils;
import wesnoth_eclipse_plugin.utils.WMLSaxHandler;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class WesnothProjectBuilder extends IncrementalProjectBuilder
{
	public WesnothProjectBuilder()
	{
		super();
	}

	protected void fullBuild(final IProgressMonitor monitor) throws CoreException
	{
		try
		{
			getProject().accept(new ResourceVisitor(monitor));
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}

	protected void incrementalBuild(IResourceDelta delta, IProgressMonitor monitor)
			throws CoreException
	{
		// the visitor does the work.
		delta.accept(new ResourceDeltaVisitor(monitor));
	}

	@SuppressWarnings("rawtypes")
	@Override
	protected IProject[] build(int kind, Map args, IProgressMonitor monitor)
			throws CoreException
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
		monitor.worked(5);

		// check for 'build.xml' existance
		if (!(new File(getProject().getLocation().toOSString() + "/build.xml").exists()))
		{
			Logger.getInstance().log("build.xml is missing. regenerating",
					"The 'build.xml' file is missing. The file will be regenerated.");

			List<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();
			params.add(new ReplaceableParameter("$$project_name", getProject().getName()));
			params.add(new ReplaceableParameter("$$project_dir_name",
					getProject().getName().equals("User Addons")? "" : getProject().getName()));
			ResourceUtils.createBuildXMLFile(
					getProject().getLocation().toOSString() + "/build.xml", params);

			getProject().refreshLocal(IResource.DEPTH_ONE, new NullProgressMonitor());

			Logger.getInstance().log("build.xml regenerated",
					"The 'build.xml' file was successfully regenerated.");
		}
		monitor.worked(2);

		// run the ant job to copy the whole project
		// in the user add-ons directory (incremental)
		monitor.subTask("Copying resources...");
		HashMap<String, String> properties = new HashMap<String, String>();
		properties.put("wesnoth.user.dir",
				Preferences.getString(Constants.P_WESNOTH_USER_DIR) + Path.SEPARATOR);
		Logger.getInstance().log("Ant result:");

		String result = AntUtils.runAnt(
				getProject().getLocation().toOSString() + "/build.xml",
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

	protected void handleRemovedResource(IResource resource)
	{
		if (resource instanceof IFile &&
			(resource.getName().toLowerCase(Locale.ENGLISH).endsWith(".cfg")))
		{
			ProjectUtils.getCacheForProject(getProject()).
					getScenarios().remove(resource.getName());
		}
	}

	protected boolean checkResource(IResource resource, IProgressMonitor monitor,
						int delta, boolean handleMainCfg)
	{
		monitor.worked(5);
		if (resource.exists() == false)
			return false;

		if (isResourceIgnored(resource))
			return false;

		if (monitor.isCanceled())
			return false;

		// config files
		if (resource instanceof IFile &&
			(resource.getName().toLowerCase(Locale.ENGLISH).endsWith(".cfg")))
		{
			if (handleMainCfg == false &&
				resource.getName().startsWith("_main.cfg"))
				return true;

			try
			{
				IFile file = (IFile) resource;
				deleteMarkers(file);

				monitor.subTask("Preprocessing...");
				PreprocessorUtils.preprocessFile(file, null);
				monitor.worked(5);

				monitor.subTask("Gathering file information...");
				ProjectCache projCache = ProjectUtils.getCacheForProject(getProject());

				if (ResourceUtils.isScenarioFile(file.getLocation().toOSString()))
				{
					WMLSaxHandler handler =  (WMLSaxHandler) ResourceUtils.
						getWMLSAXHandlerFromResource(
							PreprocessorUtils.getPreprocessedFilePath(file, false, false).toString(),
							new WMLSaxHandler());
					if (handler == null || handler.ScenarioId == null)
					{
						projCache.getScenarios().remove(file.getName());
						Logger.getInstance().logWarn("got a null scenario id" +
								"for 'scenario' file:" + file.getName());
					}
					else
					{
						Logger.getInstance().log("added scenarioId ["+handler.ScenarioId +
								"] for file: " + file.getName());
						projCache.getScenarios().put(file.getName(), handler.ScenarioId);
					}
				}
				monitor.worked(10);

				// we need to find the correct column start/end based on the current document
				// (or get that from the tool)
				IDocumentProvider provider = new TextFileDocumentProvider();
				provider.connect(file);
				IDocument document = provider.getDocument(file);

				// wmllint
				monitor.subTask("WMLLint pass...");
//				ExternalToolInvoker tool = WMLTools.runWMLLint(file.getLocation().toOSString(), true);
//				tool.waitForTool();
//
//				String[] output = StringUtils.getLines(tool.getErrorContent());
//				MarkerToken token;
//				for(String line : output)
//				{
//					token = MarkerToken.parseToken(line);
//					if (token == null)
//						continue;
//					addMarker(file, token, document);
//				}
				monitor.worked(20);

				// wmlscope
				monitor.subTask("WMLScope pass...");
//				tool = WMLTools.runWMLScope(file.getLocation().toOSString());
//				tool.waitForTool();
//				output = StringUtils.getLines(tool.getErrorContent());
//				for(String line : output)
//				{
//					token = MarkerToken.parseToken(line);
//					if (token == null)
//						continue;
//					addMarker(file, token, document);
//				}
				monitor.worked(20);

			} catch (Exception e)
			{
				Logger.getInstance().logException(e);
			}
		}
		return true;
	}

	/**
	 * Returns true if the specified resource should be skipped by the builder
	 * @param res
	 * @return
	 */
	private boolean isResourceIgnored(IResource res)
	{
		if (ProjectUtils.getPropertiesForProject(getProject()) == null)
			return false;

		String ignored = ProjectUtils.getPropertiesForProject(getProject()).getProperty("ignored");
		if (ignored == null)
			return false;

		String[] ignoredFiles = ignored.split(",");

		for (String path : ignoredFiles)
		{
			if (path.isEmpty())
				continue;

			if (StringUtils.normalizePath(WorkspaceUtils.getPathRelativeToUserDir(res))
					.contains(StringUtils.normalizePath(path)))
				return true;
		}
		return false;
	}

	/**
	 * Adds the specified MarkerToken in the selected file
	 * @param file
	 * @param token
	 * @param document
	 */
	private void addMarker(IFile file, MarkerToken token, IDocument document)
	{
		try
		{
			IMarker marker = file.createMarker(Constants.BUILDER_MARKER_TYPE);
			marker.setAttribute(IMarker.MESSAGE, token.getMessage());
			if (token.getColumnEnd() != 0)
			{
				marker.setAttribute(IMarker.CHAR_START,
					document.getLineOffset(token.getLine() - 1) + token.getColumnStart());
				marker.setAttribute(IMarker.CHAR_END,
					document.getLineOffset(token.getLine() - 1) + token.getColumnEnd());
			}
			marker.setAttribute(IMarker.LINE_NUMBER, token.getLine());
			marker.setAttribute(IMarker.SEVERITY,token.getType().toMarkerSeverity());
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}

	private void deleteMarkers(IFile file)
	{
		try
		{
			file.deleteMarkers(Constants.BUILDER_MARKER_TYPE, false, IResource.DEPTH_ZERO);
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}

	class ResourceDeltaVisitor implements IResourceDeltaVisitor
	{
		private IProgressMonitor monitor_;

		public ResourceDeltaVisitor(IProgressMonitor monitor) {
			monitor_ = monitor;
		}

		@Override
		public boolean visit(IResourceDelta delta) throws CoreException
		{
			IResource resource = delta.getResource();
			boolean visitChildren = false;
			switch (delta.getKind())
			{
			case IResourceDelta.ADDED:
				// handle added resource
				visitChildren = checkResource(resource, monitor_, delta.getKind(), false);
				break;
			case IResourceDelta.REMOVED:
				// handle removed resource
				handleRemovedResource(resource);
				visitChildren = true;
				break;
			case IResourceDelta.CHANGED:
				// handle changed resource
				visitChildren = checkResource(resource, monitor_, delta.getKind(), false);
				break;
			}

			if (resource instanceof IContainer)
			{
				// preprocess _main.cfg before all
				checkResource(((IContainer) resource).getFile(new Path("_main.cfg")),
						monitor_, delta.getKind(), true);
			}

			// return true to continue visiting children.
			return visitChildren;
		}
	}

	class ResourceVisitor implements IResourceVisitor
	{
		private IProgressMonitor monitor_;

		public ResourceVisitor(IProgressMonitor monitor) {
			monitor_ = monitor;
		}

		@Override
		public boolean visit(IResource resource)
		{
			// preprocess _main.cfg before all
			if (resource instanceof IContainer)
			{
				checkResource(((IContainer) resource).getFile(new Path("_main.cfg")),
						monitor_, -1, true);
			}
			return checkResource(resource, monitor_, -1, false);
		}
	}
}

package wesnoth_eclipse_plugin.builder;

import java.io.File;
import java.util.HashMap;
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

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.globalactions.PreprocessorActions;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;
import wesnoth_eclipse_plugin.utils.AntUtils;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class WesnothProjectBuilder extends IncrementalProjectBuilder
{

	public static final String	BUILDER_ID	= "Wesnoth_Eclipse_Plugin.projectBuilder";
	private static final String	MARKER_TYPE	= "Wesnoth_Eclipse_Plugin.configProblem";

	class SampleDeltaVisitor implements IResourceDeltaVisitor
	{
		private IProgressMonitor monitor_;
		public SampleDeltaVisitor(IProgressMonitor monitor){
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
		public SampleResourceVisitor(IProgressMonitor monitor){
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
			e.printStackTrace();
		}
	}

	protected void incrementalBuild(IResourceDelta delta, IProgressMonitor monitor) throws CoreException
	{
		// the visitor does the work.
		delta.accept(new SampleDeltaVisitor(monitor));
	}

	@Override
	protected IProject[] build(int kind, Map args, IProgressMonitor monitor) throws CoreException
	{
		System.out.println("building");
		monitor.beginTask("Building...", 100);

		if (PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_USER_DIR).isEmpty())
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Please set the wesnoth user dir before creating the content");
			return null;
		}

		// run the ant job to copy the whole project
		// in the user add-ons directory (incremental)
		if (!(new File(getProject().getLocation().toOSString() + "/build.xml").exists()))
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "The 'build.xml' file is missing. The building cannot continue.");
			// TODO: better way of handling this - maybe regenerating?
			return null;
		}

		// Ant copy
		monitor.subTask("Copying resources...");
		HashMap<String, String> properties = new HashMap<String, String>();
		properties.put("wesnoth.user.dir", PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_USER_DIR) + Path.SEPARATOR);
		System.out.println("Ant result:");
		String result = AntUtils.runAnt(getProject().getLocation().toOSString() + "/build.xml", properties, true);
		System.out.println(result);
		monitor.worked(10);

		if (result == null)
		{
			Logger.print("There was an error running the ant job.");
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "There was an error running the ant job.");
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
				e.printStackTrace();
			}
		}
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
			e.printStackTrace();
		}
	}

	private void deleteMarkers(IFile file)
	{
		try
		{
			file.deleteMarkers(MARKER_TYPE, false, IResource.DEPTH_ZERO);
		} catch (CoreException e)
		{
			e.printStackTrace();
		}
	}
}

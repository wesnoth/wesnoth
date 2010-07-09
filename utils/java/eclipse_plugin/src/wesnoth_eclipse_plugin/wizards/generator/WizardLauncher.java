/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.texteditor.AbstractTextEditor;
import org.eclipse.ui.texteditor.IDocumentProvider;
import org.eclipse.ui.texteditor.ITextEditor;

import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.WizardUtils;

public class WizardLauncher extends NewWizardTemplate
{
	WizardLauncherPage0	page0_;
	WizardLauncherPage1	page1_;
	WizardGenerator		wizard_;

	public WizardLauncher() {
		setWindowTitle("Wizard launcher");
		setNeedsProgressMonitor(true);
	}

	@Override
	public void addPages()
	{
		page0_ = new WizardLauncherPage0(selection_);
		addPage(page0_);

		page1_ = new WizardLauncherPage1();
		addPage(page1_);

		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		wizard_ = new WizardGenerator(page1_.getTagDescription() + " new wizard", page1_.getTagName(), (byte) 0);
		WizardUtils.launchWizard(wizard_, getShell(), selection_);
		if (!wizard_.isFinished())
			return false;

		IRunnableWithProgress op = new IRunnableWithProgress() {
			@Override
			public void run(IProgressMonitor monitor) throws InvocationTargetException
			{
				try
				{
					doFinish(monitor);
				} catch (CoreException e)
				{
					throw new InvocationTargetException(e);
				} finally
				{
					monitor.done();
				}
			}
		};
		try
		{
			getContainer().run(false, false, op);
		} catch (InterruptedException e)
		{
			return false;
		} catch (InvocationTargetException e)
		{
			Throwable realException = e.getTargetException();
			MessageDialog.openError(getShell(), "Error", realException.getMessage());
			return false;
		}
		return true;
	}

	private void doFinish(IProgressMonitor monitor) throws CoreException
	{
		// The file is opened in the editor -> just copy-paste the text
		if (!(page0_.getIsTargetNewFile()))
		{
			try
			{
				IEditorPart part = page0_.getEditedFile();
				if (!(part instanceof AbstractTextEditor))
					return;
				ITextEditor editor = (ITextEditor) part;
				IDocumentProvider dp = editor.getDocumentProvider();
				IDocument doc = dp.getDocument(editor.getEditorInput());
				int offset = ((ITextSelection) editor.getSelectionProvider().getSelection()).getOffset();
				doc.replace(offset, 0, wizard_.getData().toString());

			} catch (Exception e)
			{
				e.printStackTrace();
			}
			return;
		}
		final String containerName = page0_.getDirectoryName();
		final String fileName = page0_.getFileName();

		// create the file
		monitor.beginTask("Creating " + fileName, 10);
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IResource resource = root.findMember(new Path(containerName));

		IContainer container = (IContainer) resource;
		final IFile file = container.getFile(new Path(fileName));

		try
		{
			InputStream stream = new ByteArrayInputStream(wizard_.getData().toString().getBytes());

			if (file.exists())
			{
				file.setContents(stream, true, true, monitor);
			}
			else
			{
				file.create(stream, true, monitor);
			}

			stream.close();
		} catch (Exception e)
		{
			e.printStackTrace();
		}

		monitor.worked(5);
		monitor.setTaskName("Opening file for editing...");
		getShell().getDisplay().asyncExec(new Runnable() {
			@Override
			public void run()
					{
						IWorkbenchPage page = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
						try
						{
							IDE.openEditor(page, file, true);
						} catch (PartInitException e)
						{
						}
					}
		});
		monitor.worked(5);
	}
}

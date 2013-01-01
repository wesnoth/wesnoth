/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;

/**
 * Here it goes testing stuff in the plugin ( For DEBUG ONLY!)
 */
@Deprecated
public class TestHandler extends AbstractHandler
{
    @Override
    public Object execute( ExecutionEvent event ) throws ExecutionException
    {

        // IFile file =
        // ResourcesPlugin.getWorkspace().getRoot().getFileForLocation(new
        // Path("E:\\work\\java\\runtime-EclipseApplication\\A_Simple_Campaign\\scenarios\\atemplate.cfg"));
        // try
        // {
        // file.deleteMarkers(Constants.MARKER_WMLSCOPE, false,
        // IResource.DEPTH_INFINITE);
        // IMarker a= file.createMarker(Constants.MARKER_WMLLINT);
        // a.setAttribute(IMarker.LINE_NUMBER, 3);
        // a.setAttribute(IMarker.SEVERITY, IMarker.SEVERITY_WARNING);
        // }
        // catch (CoreException e)
        // {
        // e.printStackTrace();
        // }

        // DialogSettings set = new DialogSettings("proj");
        // IDialogSettings section2 = set.addNewSection("scenario");
        // section2.put("filename", "name");
        // IDialogSettings section3 = set.addNewSection("scenario2");
        // section3.put("filename", "name2");
        // try
        // {
        // set.save("D:/timo.txt");
        // }
        // catch (IOException e)
        // {
        // e.printStackTrace();
        // }
        // IWorkingSet set =
        // WorkspaceUtils.getWorkingSetManager().getWorkingSet("Default");
        // WorkingSetFilterActionGroup action = new
        // WorkingSetFilterActionGroup(Activator.getShell(),
        // null);
        // action.setWorkingSet(set);
        // DefinesSAXHandler handler = (DefinesSAXHandler)
        // ResourceUtils.getWMLSAXHandlerFromResource(
        // "d:/tmp/wesnoth_plugin/a/_MACROS_.cfg",
        // new DefinesSAXHandler());
        // System.out.println(handler.getDefines().size());
        // try
        // {
        // BuildCommand c =
        // ((BuildCommand)WorkspaceUtils.getSelectedProject().getDescription().getBuildSpec()[1]);
        // System.out.println(c);
        // }
        // catch (CoreException e)
        // {
        // e.printStackTrace();
        // }
        // IFile sel = WorkspaceUtils.getSelectedFile();
        // if (sel == null)
        // return null;
        // ExternalToolInvoker parser =
        // WMLTools.runWMLParser2(sel.getLocation().toOSString());
        // parser.waitForTool();
        // ///System.out.println(parser.getOutputContent());
        // try
        // {
        // SAXParser saxparser = SAXParserFactory.newInstance().newSAXParser();
        // WMLSaxHandler handler = new WMLSaxHandler();
        // saxparser.parse(new InputSource(new
        // StringReader(parser.getOutputContent())), handler);
        //
        // // GUIUtils.showInfoMessageBox("AA: ");
        // }
        // catch (Exception e)
        // {
        // e.printStackTrace();
        // }
        // try
        // {
        // MessageConsole con = GUIUtils.createConsole("BB", null, true);
        // OutputStream s = con.newMessageStream();
        // s.write("AAAAAAAAAAAAAA".getBytes());
        // }
        // catch (IOException e)
        // {
        // e.printStackTrace();
        // }
        // try
        // {
        // IProject proj = WorkspaceUtils.getSelectedProject();
        // String path = proj.getLocation().toOSString() + "/.wesnoth";
        // Properties config = new Properties();
        // config.setProperty("diff", "val");
        // config.storeToXML(new FileOutputStream(path), null);
        // GUIUtils.showInfoMessageBox(config.getProperty("diff"));
        // // if (proj != null)
        // // {
        // // int go = GUIUtils.showMessageBox("a", SWT.YES | SWT.NO);
        // // if (go == SWT.YES)
        // // {
        // // //org.eclipse.core.resources.textmarkerIMarker.TEXT
        // // IMarker mark = proj.createMarker("sett");
        // // mark.setAttribute("diff", "VAL");
        // // }
        // // else
        // // {
        // // IMarker[] marks = proj.findMarkers("sett", true,
        // IResource.DEPTH_INFINITE);
        // // System.out.println(marks.length);
        // // }
        // // }
        // }
        // catch (Exception e)
        // {
        // e.printStackTrace();
        // }
        // System.out.println(WorkspaceUtils.getSelectedContainer());
        // MessageConsole con = GUIUtils.createConsole("TIMO", null, true);
        // List<String> arguments = new ArrayList<String>();
        // arguments.add(Preferences.getString(Preferences.WESNOTH_WORKING_DIR));
        // if (tool == null)
        // {
        // tool = new ExternalToolInvoker("D:/timo/conapp.exe", arguments);
        // tool.runTool();
        // tool.startErrorMonitor();
        // tool.startOutputMonitor();
        // tool.waitForTool();
        // System.out.println(tool.getErrorContent());
        // System.out.println(tool.getOutputContent());
        // }
        // else
        // {
        // tool.kill(true);
        // }
        // tool.waitForTool();
        // System.out.println("Exitt");
        // String stdin = EditorUtils.getEditorDocument().get();
        // EditorUtils.replaceEditorText(WMLTools.runWMLIndent(null, stdin,
        // false, false, false));
        // IEditorReference[] files =
        // Activator.getDefault().getWorkbench().getActiveWorkbenchWindow().getPages()[0].getEditorReferences();
        // for (IEditorReference file : files)
        // {
        // if (file.isDirty())
        // file.getEditor(false).doSave(null);
        // }
        // ProgressMonitorDialog dialog = new
        // ProgressMonitorDialog(Activator.getShell());
        // try
        // {
        // dialog.run(true, true, new IRunnableWithProgress() {
        // @Override
        // public void run(IProgressMonitor monitor)
        // {
        // monitor.beginTask("Some nice progress message here ...", 100);
        // // execute the task ...
        // try
        // {
        // Thread.sleep(2000);
        // } catch (InterruptedException e)
        // {
        // Logger.getInstance().logException(e);
        // }
        // monitor.done();
        // }
        // });
        // } catch (InvocationTargetException e)
        // {
        // Logger.getInstance().logException(e);
        // } catch (InterruptedException e)
        // {
        // Logger.getInstance().logException(e);
        // }
        // UIJob job1 = new UIJob("My the job") {
        //
        // @Override
        // public IStatus runInUIThread(IProgressMonitor monitor)
        // {
        // monitor.beginTask("Some nice progress message here ...", 100);
        // // execute the task ...try
        // try
        // {
        // Thread.sleep(2000);
        // } catch (InterruptedException e)
        // {
        // Logger.getInstance().logException(e);
        // }
        // monitor.done();
        // return Status.OK_STATUS;
        // }
        // };
        // //job1.schedule();
        // WorkbenchJob job2 = new WorkbenchJob("asdasdd ") {
        //
        // @Override
        // public IStatus runInUIThread(IProgressMonitor monitor)
        // {
        // monitor.beginTask("Some nice progress message here ...", 100);
        // // execute the task ...try
        // try
        // {
        // Thread.sleep(2000);
        // } catch (InterruptedException e)
        // {
        // Logger.getInstance().logException(e);
        // }
        // monitor.done();
        // return Status.OK_STATUS;
        // }
        // };
        // job2.schedule();
        // new WorkspaceJob("My new job") {
        // @Override
        // public IStatus runInWorkspace(IProgressMonitor monitor) throws
        // CoreException
        // {
        // monitor.beginTask("Some nice progress message here ...", 100);
        // // execute the task ...try
        // try
        // {
        // Thread.sleep(2000);
        // } catch (InterruptedException e)
        // {
        // Logger.getInstance().logException(e);
        // }
        // monitor.done();
        // return Status.OK_STATUS;
        // }
        // }.schedule();
        // job.schedule();
        return null;
    }
}

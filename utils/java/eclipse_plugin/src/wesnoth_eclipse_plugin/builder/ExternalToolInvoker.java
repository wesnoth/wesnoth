/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.builder;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.console.MessageConsole;
import org.eclipse.ui.console.MessageConsoleStream;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.utils.MyRunnable;

//TODO: needs REWRITE !! -> use streams rather than a hundred of parameters
public class ExternalToolInvoker
{
	private Process			process_;
	private ProcessBuilder	processBuilder_;

	private Thread			processThread_;
	private Thread			attachedThread_;

	private BufferedReader	bufferedReaderOutput_;
	private BufferedReader	bufferedReaderError_;

	private String			outputContent_;
	private String			errorContent_;

	/**
	 * Creates an external tool invoker with specified options
	 *
	 * @param fileName the file name to be invoked
	 * @param arguments the arguments passed to the file
	 * @param useThread true if the process will run in a thread
	 */
	public ExternalToolInvoker(String fileName, List<String> arguments, boolean useThread) {
		List<String> commandline = new ArrayList<String>();
		commandline.add(fileName);
		if (arguments != null)
			commandline.addAll(arguments);

		processBuilder_ = new ProcessBuilder(commandline);
		if (useThread)
		{
			processThread_ = new Thread(new Runnable() {
				@Override
				public void run()
				{
					try
					{
						process_ = processBuilder_.start();

						bufferedReaderOutput_ = new BufferedReader(new InputStreamReader(process_.getInputStream()));
						bufferedReaderError_ = new BufferedReader(new InputStreamReader(process_.getErrorStream()));
					} catch (IOException e)
					{
						Logger.getInstance().logException(e);
					}
				}
			});
		}
	}

	public void run()
	{
		try
		{
			if (processThread_ == null)
			{
				process_ = processBuilder_.start();

				bufferedReaderOutput_ = new BufferedReader(new InputStreamReader(process_.getInputStream()));
				bufferedReaderError_ = new BufferedReader(new InputStreamReader(process_.getErrorStream()));
			}
			else
				processThread_.start();
		} catch (IOException e)
		{
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Waits for the current tool, and returns the return value
	 *
	 * @return the return value of the tool
	 */
	public int waitForTool()
	{
		if (process_ == null)
			return 0;

		try
		{
			return process_.waitFor();
		} catch (InterruptedException e)
		{
			return -1;
		}
	}

	public String getOutputContent()
	{
		return outputContent_;
	}

	public String readOutputLine()
	{
		if (process_ == null || bufferedReaderOutput_ == null)
			return null;

		try
		{
			return bufferedReaderOutput_.readLine();
		} catch (IOException e)
		{
			Logger.getInstance().logException(e);
			return null;
		}
	}

	public String getErrorContent()
	{
		return errorContent_;
	}

	public String readErrorLine()
	{
		if (process_ == null || bufferedReaderError_ == null)
			return null;

		try
		{
			return bufferedReaderError_.readLine();
		} catch (IOException e)
		{
			Logger.getInstance().logException(e);
			return null;
		}
	}

	public OutputStream getOutputStream()
	{
		if (process_ == null)
			return null;

		return process_.getOutputStream();
	}

	public InputStream getInputStream()
	{
		if (process_ == null)
			return null;

		return process_.getInputStream();
	}

	public InputStream getErrorStream()
	{
		if (process_ == null)
			return null;

		return process_.getErrorStream();
	}

	public boolean processEnded()
	{
		try
		{
			if (process_ != null)
				process_.exitValue();
			else
				return false;
		} catch (IllegalThreadStateException e)
		{
			// the process hasn't exited
			return false;
		}
		return true;
	}

	public void kill()
	{
		if (process_ != null)
			process_.destroy();
		if (processThread_ != null)
			processThread_.interrupt();
	}

	/**
	 * Gets the owned thread used to run the process.
	 * This is non-null if the tool was invoked with "useThread=true"
	 *
	 * @return
	 */
	public Thread getOwnThread()
	{
		return processThread_;
	}

	/**
	 * Gets the attached thread, usually when someone runs this tool in another thread
	 *
	 * @return
	 */
	public Thread getAttachedThread()
	{
		return attachedThread_;
	}

	/**
	 * Sets the attached thread
	 *
	 * @param thread
	 */
	public void setAttachedThread(Thread thread)
	{
		attachedThread_ = thread;
	}

	/**
	 * Launches the specified tool, with the specified argument list
	 *
	 * @param fileName the full path to the executable to be launched
	 * @param args the arguments list
	 * @param outputFlags a composition of flags used for output
	 * @param useThread true to launch the tool on a separate thread.
	 *        If this is false the method will wait for the tool to end
	 * @param workbenchWindow the workbench window used to show messages
	 *        (if null no messages will be triggered)
	 * @return
	 */
	public static ExternalToolInvoker launchTool(final String fileName, final List<String> args,
			final int outputFlags, final boolean useThread)
	{
		final ExternalToolInvoker toolInvoker = new ExternalToolInvoker(fileName, args, useThread);

		System.out.println("Tool args: " + args);

		MessageConsoleStream stream = null;
		if ((outputFlags & Constants.TI_SHOW_OUTPUT_USER) == Constants.TI_SHOW_OUTPUT_USER)
		{
			MessageConsole console = new MessageConsole("", null);
			console.activate();
			ConsolePlugin.getDefault().getConsoleManager().addConsoles(new IConsole[] { console });
			stream = console.newMessageStream();
		}
		if (useThread)
		{
			toolInvoker.run();
			Thread outputStreamThread = new Thread(new MyRunnable<MessageConsoleStream>(stream) {
				@Override
				public void run()
				{
					try
					{
						String line = "";
						while (!toolInvoker.processEnded())
						{
							if ((line = toolInvoker.readOutputLine()) != null)
							{
								System.out.println(line);
								if (runnableObject_ != null)
									runnableObject_.write(line + "\n");
							}
						}
					} catch (IOException e)
					{
						Logger.getInstance().logException(e);
					}
				}
			});
			Thread errorStreamThread = new Thread(new MyRunnable<MessageConsoleStream>(stream) {
				@Override
				public void run()
				{
					try
					{
						String line = "";
						while (!toolInvoker.processEnded())
						{
							if ((line = toolInvoker.readErrorLine()) != null)
							{
								System.out.println(line);
								if (runnableObject_ != null)
									runnableObject_.write(line + "\n");
							}
						}
						System.out.println("tool exited.");

						//if (toolInvoker.waitFor() != 0)
						//{
						//	GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
						//			"The tool returned a non-zero value.");
						//}
					} catch (IOException e)
					{
						Logger.getInstance().logException(e);
					}
				}
			});
			outputStreamThread.start();
			errorStreamThread.start();
		}
		else
		{
			try
			{
				if ((outputFlags & Constants.TI_SHOW_OUTPUT) == Constants.TI_SHOW_OUTPUT)
				{
					String line = "";
					while ((line = toolInvoker.readOutputLine()) != null)
					{
						if (stream != null)
							stream.write(line + "\n");
						System.out.println(line);
					}
					while ((line = toolInvoker.readErrorLine()) != null)
					{
						if (stream != null)
							stream.write(line + "\n");
						System.out.println(line);
					}
					System.out.println("tool exited.");
				}
			} catch (IOException e)
			{
				Logger.getInstance().logException(e);
			}
		}
		return toolInvoker;
	}
}

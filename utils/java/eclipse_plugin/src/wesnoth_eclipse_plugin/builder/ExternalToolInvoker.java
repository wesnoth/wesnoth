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

public class ExternalToolInvoker
{
	private Process process_;
	private ProcessBuilder processBuilder_;

	private BufferedReader bufferedReaderOutput_;
	private BufferedReader bufferedReaderError_;

	// Thread for monitoring stdout
	private Thread monitorOutputThread_;
	private String outputContent_ 		= "";

	// Thread for monitoring stderr
	private Thread monitorErrorThread_;
	private String errorContent_ 		= "";

	/**
	 * Creates an external tool invoker with specified options
	 *
	 * @param fileName the file name to be invoked
	 * @param arguments the arguments passed to the file
	 */
	public ExternalToolInvoker(String fileName, List<String> arguments) {
		List<String> commandline = new ArrayList<String>();
		commandline.add(fileName);
		if (arguments != null)
			commandline.addAll(arguments);

		processBuilder_ = new ProcessBuilder(commandline);
		Logger.getInstance().log(String.format("Invoking tool %s with args: %s\n",
				fileName, arguments));
	}

	/**
	 * Runs the current tool
	 */
	public void runTool()
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

	/**
	 * Waits for the current tool, and returns the return value
	 *
	 * if the process is null (not started) => 0
	 * if there was an error => -1
	 *
	 * @return the return value of the tool
	 */
	public int waitForTool()
	{
		try
		{
			if (process_ == null)
				return 0;
			return process_.waitFor();
		} catch (InterruptedException e)
		{
			return -1;
		}
	}

	/**
	 * Reads a line from the stdout.
	 * Returns null if process wasn't started or an exception was thrown
	 *
	 * @return
	 */
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

	/**
	 * Reads a line from the stderr.
	 * Returns null if process wasn't started or an exception was thrown
	 *
	 * @return
	 */
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

	/**
	 * Starts a new thread monitoring stderr.
	 * All "Error" output will be available to be read from <code>getErrorContent()</code>
	 */
	public void startErrorMonitor()
	{
		monitorErrorThread_ = new Thread(new Runnable() {
			@Override
			public void run()
			{
				String line = "";
				while((line = readErrorLine()) != null)
				{
					errorContent_ += (line + "\n");
				}
			}
		});
		monitorErrorThread_.start();
	}

	/**
	 * Starts a new thread monitoring stdout.
	 * All "Output" output will be available to be read from <code>getOutputContent()</code>
	 */
	public void startOutputMonitor()
	{
		monitorOutputThread_ = new Thread(new Runnable() {
			@Override
			public void run()
			{
				String line = "";
				while((line = readOutputLine()) != null)
				{
					outputContent_ += (line + "\n");
				}
			}
		});
		monitorOutputThread_.start();
	}

	/**
	 * Gets the content (as String) of the stderr,
	 * if the caller started "startErrorMonitor"
	 * @return
	 */
	public String getErrorContent()
	{
		return errorContent_;
	}

	/**
	 * Gets the content (as String) of the stdout,
	 * if the caller started "startOutputMonitor"
	 * @return
	 */
	public String getOutputContent()
	{
		return outputContent_;
	}

	/**
	 * Returns the OutputStream
	 * @return
	 */
	public OutputStream getStdin()
	{
		if (process_ == null)
			return null;

		return process_.getOutputStream();
	}

	/**
	 * Returns the InputStream
	 * @return
	 */
	public InputStream getStdout()
	{
		if (process_ == null)
			return null;

		return process_.getInputStream();
	}

	/**
	 * Returns the ErrorStream
	 * @return
	 */
	public InputStream getStderr()
	{
		if (process_ == null)
			return null;

		return process_.getErrorStream();
	}

	/**
	 * Returns true if the process ended
	 * @return
	 */
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

	/**
	 * Kills the current opened tool. No effect is tool is already killed
	 *
	 * @param waitForKilling true to wait until the process is killed, so when the call
	 * returns the process will be already finished (it has "exitValue")
	 */
	public void kill(boolean waitForKilling)
	{
		if (process_ != null)
		{
			process_.destroy();
			if (waitForKilling)
			{
				try
				{
					process_.waitFor();
				} catch (InterruptedException e)
				{
				}
			}
			monitorErrorThread_.interrupt();
			monitorOutputThread_.interrupt();
		}
	}

	public static ExternalToolInvoker launchTool(final String fileName, final List<String> args)
			//final OutputStream )
	{
		final ExternalToolInvoker toolInvoker = new ExternalToolInvoker(fileName, args);

		return toolInvoker;
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
		final ExternalToolInvoker toolInvoker = new ExternalToolInvoker(fileName, args);

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
			toolInvoker.runTool();
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

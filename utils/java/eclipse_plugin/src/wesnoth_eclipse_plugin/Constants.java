/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin;

import org.eclipse.xtext.ui.XtextProjectHelper;

/**
 * Constant definitions for plug-in
 */
public class Constants
{
	/** Preferences Constants **/
	public static final String	P_WESNOTH_EXEC_PATH		= "wesnoth_exec_path";
	public static final String	P_WESNOTH_WORKING_DIR	= "wesnoth_working_dir";
	public static final String	P_WESNOTH_WMLTOOLS_DIR	= "wesnoth_wmltools_dir";

	public static final String	P_WESNOTH_USER_DIR		= "wesnoth_user_dir";

	/** Wizards Constants **/
	public static final int		WIZ_TextBoxHeight		= 21;
	public static final int		WIZ_MaxTextBoxesOnPage	= 10;
	public static final int		WIZ_MaxGroupsOnPage		= 4;
	public static final int		WIZ_MaxWizardPageHeight	= 220;

	/** Builder Constants **/
	public static final String BUIILDER_WESNOTH 	= "Wesnoth_Eclipse_Plugin.projectBuilder";
	public static final String BUILDER_XTEXT 		= XtextProjectHelper.BUILDER_ID;
	public static final String BUILDER_MARKER_TYPE= "Wesnoth_Eclipse_Plugin.configProblem";

	/** Nature Constants **/
	public static final String NATURE_WESNOTH		= "Wesnoth_Eclipse_Plugin.wesnothNature";
	public static final String NATURE_XTEXT		= XtextProjectHelper.NATURE_ID;
}

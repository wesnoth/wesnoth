/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import java.util.Locale;

import org.eclipse.xtext.ui.XtextProjectHelper;

/**
 * Constant definitions for plug-in
 */
public class Constants
{
	/** Plugin related */
	public static final String PLUGIN_FULL_PATH =
		Constants.class.getProtectionDomain().getCodeSource().getLocation().getPath() + "/"; //$NON-NLS-1$
	public static final String PLUGIN_ID  = "Wesnoth_Eclipse_Plugin"; //$NON-NLS-1$

	/**
	 * The boolean value whether this machine is running windows or not
	 */
	public static boolean IS_WINDOWS_MACHINE =
		System.getProperty("os.name").toLowerCase(Locale.ENGLISH).contains("windows"); //$NON-NLS-1$ //$NON-NLS-2$

	/**
	 * The boolean value whether this machine is running Machintosh or not
	 */
	public static boolean IS_MAC_MACHINE =
		System.getProperty("os.name").toLowerCase(Locale.ENGLISH).contains("mac"); //$NON-NLS-1$ //$NON-NLS-2$
	public static String MACHINE_OS =
		System.getProperty("os.name").toLowerCase(Locale.ENGLISH); //$NON-NLS-1$

	/** Preferences Constants **/
	public static final String	P_WESNOTH_EXEC_PATH     = "wesnoth_exec_path"; //$NON-NLS-1$
	public static final String	P_WESNOTH_WORKING_DIR   = "wesnoth_working_dir"; //$NON-NLS-1$
	public static final String	P_WESNOTH_WMLTOOLS_DIR  = "wesnoth_wmltools_dir"; //$NON-NLS-1$

	public static final String	P_WESNOTH_USER_DIR      = "wesnoth_user_dir"; //$NON-NLS-1$

	public static final String 	P_PYTHON_PATH           = "python_path"; //$NON-NLS-1$

	/** WML Tools preferences constants **/
	public static final String	P_WMLINDENT_VERBOSE     = "wmlindent_verbose"; //$NON-NLS-1$
	public static final String	P_WMLINDENT_DRYRUN      = "wmlindent_dry_run"; //$NON-NLS-1$

	public static final String	P_WMLLINT_DRYRUN        = "wmllint_dry_run"; //$NON-NLS-1$
	public static final String	P_WMLLINT_SPELL_CHECK   = "wmllint_spell_check"; //$NON-NLS-1$
	public static final String	P_WMLLINT_VERBOSE_LEVEL = "wmllint_verbose_level"; //$NON-NLS-1$

	public static final String	P_WMLSCOPE_VERBOSE_LEVEL= "wmlscope_verbose_level"; //$NON-NLS-1$
	public static final String	P_WMLSCOPE_COLLISIONS   = "wmlscope_collisions"; //$NON-NLS-1$

	/** Wesnoth addon uploader preferences */
	public static final String 	P_WAU_PASSWORD          = "wau_password"; //$NON-NLS-1$
	public static final String 	P_WAU_VERBOSE           = "wau_verbose"; //$NON-NLS-1$
	public static final String 	P_WAU_ADDRESS           = "wau_address"; //$NON-NLS-1$
	public static final String 	P_WAU_PORT              = "wau_port"; //$NON-NLS-1$

	/** Advanced preferences */
	public static final String	P_ADV_NO_TERRAIN_GFX	= "adv_no_terrain_gfx"; //$NON-NLS-1$

	/** Wizards Constants **/
	public static final int		WIZ_TextBoxHeight       = 21;
	public static final int		WIZ_MaxTextBoxesOnPage  = 10;
	public static final int		WIZ_MaxGroupsOnPage     = 4;
	public static final int		WIZ_MaxWizardPageHeight = 220;

	/** Builder Constants **/
	public static final String BUIILDER_WESNOTH     = "Wesnoth_Eclipse_Plugin.projectBuilder"; //$NON-NLS-1$
	public static final String BUILDER_XTEXT        = XtextProjectHelper.BUILDER_ID;

	/** Markers **/
	public static final String MARKER_WMLSCOPE      = "org.wesnoth.marker.wmlscope"; //$NON-NLS-1$
	public static final String MARKER_WMLLINT       = "org.wesnoth.marker.wmllint"; //$NON-NLS-1$

	/** Nature Constants **/
	public static final String NATURE_WESNOTH       = "Wesnoth_Eclipse_Plugin.wesnothNature"; //$NON-NLS-1$
	public static final String NATURE_XTEXT         = XtextProjectHelper.NATURE_ID;

	/** Templates related */
	public static final String TEMPLATES_FILENAME   = "templatesIndex.txt"; //$NON-NLS-1$
}

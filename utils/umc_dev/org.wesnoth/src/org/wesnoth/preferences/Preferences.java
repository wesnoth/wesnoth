/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import java.util.HashMap;
import java.util.Map;

import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;
import org.eclipse.jface.preference.IPreferenceStore;

import org.wesnoth.WesnothPlugin;
import org.wesnoth.utils.StringUtils;

/**
 * Class used to initialize default preference values.
 */
public class Preferences extends AbstractPreferenceInitializer
{
    /**
     * Wesnoth executable path preference name
     */
    public static final String                  WESNOTH_EXEC_PATH      = "wesnoth_exec_path";            //$NON-NLS-1$
    /**
     * Wesnoth working dir ( contains the data directory ) preference name
     */
    public static final String                  WESNOTH_WORKING_DIR    = "wesnoth_working_dir";          //$NON-NLS-1$
    /**
     * Wesnoth wml tools directory ( data/tools ) preference name
     */
    public static final String                  WESNOTH_WMLTOOLS_DIR   = "wesnoth_wmltools_dir";         //$NON-NLS-1$

    /**
     * Wesnoth user's directory ( where preferences, maps, etc are stored )
     * preference name
     */
    public static final String                  WESNOTH_USER_DIR       = "wesnoth_user_dir";             //$NON-NLS-1$

    /**
     * The path to the python binary preference name
     */
    public static final String                  PYTHON_PATH            = "python_path";                  //$NON-NLS-1$

    /**
     * Verbose flag for WMLIndent preference name
     */
    public static final String                  WMLINDENT_VERBOSE      = "wmlindent_verbose";            //$NON-NLS-1$
    /**
     * Dryrun flag for wmlindent preference name
     */
    public static final String                  WMLINDENT_DRYRUN       = "wmlindent_dry_run";            //$NON-NLS-1$

    /**
     * Dryrun flag for wmllint preference name
     */
    public static final String                  WMLLINT_DRYRUN         = "wmllint_dry_run";              //$NON-NLS-1$
    /**
     * Spell Check flag for wmllint preference name
     */
    public static final String                  WMLLINT_SPELL_CHECK    = "wmllint_spell_check";          //$NON-NLS-1$
    /**
     * Verbose level for wmllint preference name
     */
    public static final String                  WMLLINT_VERBOSE_LEVEL  = "wmllint_verbose_level";        //$NON-NLS-1$

    /**
     * Verbose level for wmlscope preference name
     */
    public static final String                  WMLSCOPE_VERBOSE_LEVEL = "wmlscope_verbose_level";       //$NON-NLS-1$
    /**
     * Collisions flag for wmlscope preference name
     */
    public static final String                  WMLSCOPE_COLLISIONS    = "wmlscope_collisions";          //$NON-NLS-1$

    /**
     * Wesnoth addon manager password preference name
     */
    public static final String                  ADDON_MANAGER_PASSWORD = "wau_password";                 //$NON-NLS-1$
    /**
     * Wesnoth addon manager verbose flag preference name
     */
    public static final String                  ADDON_MANAGER_VERBOSE  = "wau_verbose";                  //$NON-NLS-1$
    /**
     * Wesnoth addon manager default address preference name
     */
    public static final String                  ADDON_MANAGER_ADDRESS  = "wau_address";                  //$NON-NLS-1$
    /**
     * Wesnoth addon manager default port preference name
     */
    public static final String                  ADDON_MANAGER_PORT     = "wau_port";                     //$NON-NLS-1$

    /**
     * No terraing gfx preprocessor flag preference name
     */
    public static final String                  NO_TERRAIN_GFX         = "adv_no_terrain_gfx";           //$NON-NLS-1$
    /**
     * WML Validation flag ( in-editor ) preference name
     */
    public static final String                  WML_VALIDATION         = "adv_wml_validation";           //$NON-NLS-1$

    /**
     * The default install name preference name
     */
    public static final String                  INST_DEFAULT_INSTALL   = "inst_default";                 //$NON-NLS-1$
    /**
     * Install list preference name
     */
    public static final String                  INST_INSTALL_LIST      = "inst_list";                    //$NON-NLS-1$
    /**
     * Install name prefix preference name
     */
    public static final String                  INST_NAME_PREFIX       = "inst_name";                    //$NON-NLS-1$


    /**
     * A map for storing the paths for the current session
     */
    protected final static Map< String, Paths > paths_                 = new HashMap< String, Paths >( );

    /**
     * Initializes the preferences to their default values
     */
    public static void initializeToDefault( )
    {
        IPreferenceStore store = WesnothPlugin.getDefault( )
            .getPreferenceStore( );
        // general settings
        store.setDefault( WESNOTH_EXEC_PATH, "" ); //$NON-NLS-1$
        store.setDefault( WESNOTH_WORKING_DIR, "" ); //$NON-NLS-1$
        store.setDefault( WESNOTH_USER_DIR, "" ); //$NON-NLS-1$
        store.setDefault( WESNOTH_WMLTOOLS_DIR, "" ); //$NON-NLS-1$
        store.setDefault( PYTHON_PATH, "" ); //$NON-NLS-1$

        // wml tools
        store.setDefault( WMLINDENT_VERBOSE, true );
        store.setDefault( WMLINDENT_DRYRUN, true );

        store.setDefault( WMLLINT_DRYRUN, true );
        store.setDefault( WMLLINT_SPELL_CHECK, false );
        store.setDefault( WMLLINT_VERBOSE_LEVEL, 0 );

        store.setDefault( WMLSCOPE_VERBOSE_LEVEL, 0 );
        store.setDefault( WMLSCOPE_COLLISIONS, false );

        // upload manager
        store.setDefault( ADDON_MANAGER_PASSWORD, "" ); //$NON-NLS-1$
        store.setDefault( ADDON_MANAGER_VERBOSE, false );
        store.setDefault( ADDON_MANAGER_ADDRESS, "add-ons.wesnoth.org" ); //$NON-NLS-1$
        store.setDefault( ADDON_MANAGER_PORT, 15002 );

        // advanced
        store.setDefault( NO_TERRAIN_GFX, true );
        store.setDefault( WML_VALIDATION, false );

        // installs
        store.setDefault( INST_DEFAULT_INSTALL, "" ); //$NON-NLS-1$
        store.setDefault( INST_INSTALL_LIST, "" ); //$NON-NLS-1$
    }

    @Override
    public void initializeDefaultPreferences( )
    {
        initializeToDefault( );
    }

    /**
     * @return The preferences store of the plugin
     */
    public static IPreferenceStore getPreferences( )
    {
        return WesnothPlugin.getDefault( ).getPreferenceStore( );
    }

    /**
     * Returns the contents of the specified preference as a string
     * or empty string ("") if there is no such preference set
     * 
     * @param prefName
     *        The name of the preference to get
     * @return The string value of the preference
     */
    public static String getString( String prefName )
    {
        return getPreferences( ).getString( prefName );
    }

    /**
     * Sets the contents of the specified preference name
     * 
     * @param prefName
     *        The preference's content to set
     * @param newValue
     *        The new value to set
     */
    public static void setString( String prefName, String newValue )
    {
        getPreferences( ).setValue( prefName, newValue );
    }

    /**
     * Returns the contents of the specified preference as an int
     * or zero (0) if there is no such preference set
     * 
     * @param prefName
     *        The name of the preference to get
     * @return The integer value of the preference
     */
    public static int getInt( String prefName )
    {
        return getPreferences( ).getInt( prefName );
    }

    /**
     * Returns the contents of the specified preference as a boolean
     * or false if there is no such preference set
     * 
     * @param prefName
     *        The name of the preference to get
     * @return The boolean value of the preference
     */
    public static boolean getBool( String prefName )
    {
        return getPreferences( ).getBoolean( prefName );
    }

    /**
     * Gets the install preference prefix for the specified install name.
     * 
     * @param installName
     *        The name of the install. If the parameter is null,
     *        the default install prefix is returned
     * @return The install prefix string
     */
    public static String getInstallPrefix( String installName )
    {
        if( StringUtils.isNullOrEmpty( installName ) ) {
            installName = getDefaultInstallName( );
        }

        return "inst_" + installName + "_"; //$NON-NLS-1$  //$NON-NLS-2$
    }

    /**
     * Returns the name of the default install
     * 
     * @return Returns the name of the default install
     */
    public static String getDefaultInstallName( )
    {
        return getString( Preferences.INST_DEFAULT_INSTALL );
    }

    /**
     * Sets the default install name
     * 
     * @param newInstallName
     *        The new install name
     */
    public static void setDefaultInstallName( String newInstallName )
    {
        getPreferences( ).setValue( Preferences.INST_DEFAULT_INSTALL,
            newInstallName );
    }

    /**
     * Returns a new Paths object based on the specified install name
     * 
     * @param installName
     *        The install name used for the paths
     * @return A new Paths object
     */
    public static Paths getPaths( String installName )
    {
        // no null allowed -> fallback to ""
        if( installName == null ) {
            installName = ""; //$NON-NLS-1$
        }

        Paths paths = paths_.get( installName );
        if( paths == null ) {
            paths = new Paths( installName, getInstallPrefix( installName ) );
            paths_.put( installName, paths );
        }

        return paths;
    }

    /**
     * Helper for accessing the paths used in the plugin based
     * on the install
     */
    public static class Paths
    {
        private String installPrefix_;
        private String installName_;

        /**
         * Creates a new Paths instance
         * 
         * @param installName
         *        The name of the install used in this instance
         * @param installPrefix
         *        The instal prefix for the preferences
         */
        public Paths( String installName, String installPrefix )
        {
            installPrefix_ = installPrefix;
            installName_ = installName;
        }

        /**
         * Returns the install name associated with this instance
         * 
         * @return A string representing the install name
         */
        public String getInstallName( )
        {
            return installName_;
        }

        /**
         * Returns the addons directory
         * 
         * @return Returns the addons directory
         */
        public String getAddonsDir( )
        {
            return getUserDataDir( ) + "add-ons/"; //$NON-NLS-1$
        }

        /**
         * Returns the data user directory
         * 
         * @return Returns the data user directory
         */
        public String getUserDataDir( )
        {
            return getUserDir( ) + "data/";
        }

        /**
         * Returns the campaign directory
         * 
         * @return Returns the campaign directory
         */
        public String getCampaignDir( )
        {
            return getWorkingDir( ) + "data/campaigns/"; //$NON-NLS-1$
        }

        /**
         * Returns the 'data/core' directory
         * 
         * @return Returns the 'data/core' directory
         */
        public String getCoreDir( )
        {
            return getWorkingDir( ) + "data/core/"; //$NON-NLS-1$
        }

        /**
         * Returns the 'data/core' directory as a Path variable
         * 
         * @return Returns the 'data/core' directory
         */
        public Path getCoreDirPath( )
        {
            return new Path( getCoreDir( ) );
        }

        /**
         * Returns the <b>schema.cfg</b> file path
         * 
         * @return Returns the schema.cfg file path
         */
        public String getSchemaPath( )
        {
            return getWorkingDir( ) + "data/schema.cfg"; //$NON-NLS-1$
        }

        /**
         * Returns the user's directory
         * 
         * @return Returns the user's directory
         */
        public String getUserDir( )
        {
            return getString( installPrefix_ + Preferences.WESNOTH_USER_DIR )
                .replace( '\\', '/' ) + IPath.SEPARATOR;
        }

        /**
         * Sets the user directory
         * 
         * @param userDir
         *        The new user directory path
         */
        public void setUserDir( String userDir )
        {
            setString( installPrefix_ + Preferences.WESNOTH_USER_DIR, userDir );
        }

        /**
         * Returns the working directory that contains the
         * <b>data</b> folder
         * 
         * @return Returns the working directory
         */
        public String getWorkingDir( )
        {
            return getString( installPrefix_ + Preferences.WESNOTH_WORKING_DIR )
                .replace( '\\', '/' ) + IPath.SEPARATOR;
        }

        /**
         * Sets the working directory that contains the
         * <b>data</b> folder
         * 
         * @param workingDir
         *        The new directory
         */
        public void setWorkingDir( String workingDir )
        {
            setString( installPrefix_ + Preferences.WESNOTH_WORKING_DIR,
                workingDir );
        }

        /**
         * Returns the directory that contains the wml tools
         * ( wmlscope, wmllint, wmlindent, wesnoth_addons_manager, etc)
         * 
         * @return Returns the directory that contains the wml tools
         */
        public String getWMLToolsDir( )
        {
            return getString( installPrefix_ + Preferences.WESNOTH_WMLTOOLS_DIR )
                .replace( '\\', '/' )
                + IPath.SEPARATOR;
        }

        /**
         * Sets the directory that contains the wml tools
         * ( wmlscope, wmllint, wmlindent, wesnoth_addons_manager, etc)
         * 
         * @param wmlToolsDir
         *        The new directory
         */
        public void setWMLToolsDir( String wmlToolsDir )
        {
            setString( installPrefix_ + Preferences.WESNOTH_WMLTOOLS_DIR,
                wmlToolsDir );
        }

        /**
         * Returns the path to the wesnoth executable
         * 
         * @return Returns the path to the wesnoth executable
         */
        public String getWesnothExecutablePath( )
        {
            return getString( installPrefix_ + Preferences.WESNOTH_EXEC_PATH )
                .replace( '\\', '/' );
        }

        /**
         * Sets the path to the wesnoth executable
         * 
         * @param executablePath
         *        The new path
         */
        public void setWesnothExecutablePath( String executablePath )
        {
            setString( installPrefix_ + Preferences.WESNOTH_EXEC_PATH,
                executablePath );
        }
    }
}

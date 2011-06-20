/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.preferences.Preferences;

public class WesnothInstallsUtils
{
    public static class WesnothInstall
    {
        public String Name;
        public String Version;

        public WesnothInstall(String name, String version)
        {
            Name = name;
            Version = version;
        }
    }

    /**
     * Returns a list of the current wesnoth installations available
     * in the preferences store
     * @return A list with Wesnoth Installs
     */
    public static List<WesnothInstall> getInstalls()
    {
        List<WesnothInstall> installsList = new ArrayList<WesnothInstall>();
        // unpack installs
        String[] installs = Preferences.getString( Constants.P_INST_INSTALL_LIST ).split( ";" ); //$NON-NLS-1$
        for ( String str : installs ){
            if ( str.isEmpty() )
                continue;

            String[] tokens = str.split( ":" ); //$NON-NLS-1$

            if ( tokens.length != 2 ) {
                Logger.getInstance().logError( "invalid install [" + str + "] in installs list." ); //$NON-NLS-1$
                continue;
            }

            installsList.add( new WesnothInstall( tokens[0], tokens[1] ) );
        }

        return installsList;
    }

    /**
     * Sets the specified Installs list in the preferences store
     * @param installsList The list to replace / set the installs list
     */
    public static void setInstalls( Collection<WesnothInstall> installsList )
    {
        // pack back the installs
        StringBuilder installs = new StringBuilder();
        for ( WesnothInstall install : installsList ) {
            // don't save the default install
            if ( install.Name.equals( "Default" ) )
                continue;

            if ( installs.length() > 0 )
                installs.append( ";" ); //$NON-NLS-1$

            installs.append( install.Name );
            installs.append( ":" ); //$NON-NLS-1$
            installs.append( install.Version );
        }
        Preferences.getPreferences().setValue( Constants.P_INST_INSTALL_LIST, installs.toString() );
    }
}

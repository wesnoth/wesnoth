/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import java.util.Locale;

/**
 * Constant definitions for plug-in
 */
public class Constants
{
    /**
     * The Operating System string representation of the current machine.
     */
    public static final String  MACHINE_OS         = System
                                                       .getProperty( "os.name" ).toLowerCase( Locale.ENGLISH ); //$NON-NLS-1$
    /**
     * The boolean value whether this machine is running windows or not
     */
    public static final boolean IS_WINDOWS_MACHINE = MACHINE_OS
                                                       .contains( "windows" );                                 //$NON-NLS-1$
    /**
     * The boolean value whether this machine is running Machintosh or not
     */
    public static final boolean IS_MAC_MACHINE     = MACHINE_OS
                                                       .contains( "mac" );                                     //$NON-NLS-1$

    /** The WMLScope marker ID */
    public static final String  MARKER_WMLSCOPE    = "org.wesnoth.marker.wmlscope";                            //$NON-NLS-1$
    /** The WMLLint marker ID */
    public static final String  MARKER_WMLLINT     = "org.wesnoth.marker.wmllint";                             //$NON-NLS-1$
}

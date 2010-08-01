/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import wesnoth_eclipse_plugin.schema.SchemaParser;

/**
 * Initialization support for running Xtext languages
 * without equinox extension registry
 */
public class WMLStandaloneSetup extends WMLStandaloneSetupGenerated
{
	public static void doSetup() {
		new WMLStandaloneSetup().createInjectorAndDoEMFRegistration();
		SchemaParser.getInstance().parseSchema(false);
	}
}


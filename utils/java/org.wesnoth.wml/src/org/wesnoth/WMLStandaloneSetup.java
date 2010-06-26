
package org.wesnoth;

/**
 * Initialization support for running Xtext languages 
 * without equinox extension registry
 */
public class WMLStandaloneSetup extends WMLStandaloneSetupGenerated{

	public static void doSetup() {
		new WMLStandaloneSetup().createInjectorAndDoEMFRegistration();
	}
}


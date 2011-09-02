/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import org.wesnoth.tests.pde.ProjectDependencyListTests;

@RunWith( Suite.class )
@Suite.SuiteClasses( { ProjectDependencyListTests.class } )
public class WMLPDETestsSuite
{

}

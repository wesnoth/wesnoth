/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.ecore.EFactory;

/**
 * <!-- begin-user-doc -->
 * The <b>Factory</b> for the model.
 * It provides a create method for each non-abstract class of the model.
 * <!-- end-user-doc -->
 * @see org.wesnoth.wml.WmlPackage
 * @generated
 */
public interface WmlFactory extends EFactory
{
  /**
   * The singleton instance of the factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  WmlFactory eINSTANCE = org.wesnoth.wml.impl.WmlFactoryImpl.init();

  /**
   * Returns a new object of class '<em>WML Root</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Root</em>'.
   * @generated
   */
  WMLRoot createWMLRoot();

  /**
   * Returns a new object of class '<em>WML Grammar Element</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Grammar Element</em>'.
   * @generated
   */
  WMLGrammarElement createWMLGrammarElement();

  /**
   * Returns a new object of class '<em>WML Tag</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Tag</em>'.
   * @generated
   */
  WMLTag createWMLTag();

  /**
   * Returns a new object of class '<em>WML Key</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Key</em>'.
   * @generated
   */
  WMLKey createWMLKey();

  /**
   * Returns a new object of class '<em>WML Key Value</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Key Value</em>'.
   * @generated
   */
  WMLKeyValue createWMLKeyValue();

  /**
   * Returns a new object of class '<em>WML Macro Call</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Macro Call</em>'.
   * @generated
   */
  WMLMacroCall createWMLMacroCall();

  /**
   * Returns a new object of class '<em>WML Macro Call Parameter</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Macro Call Parameter</em>'.
   * @generated
   */
  WMLMacroCallParameter createWMLMacroCallParameter();

  /**
   * Returns a new object of class '<em>WML Array Call</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Array Call</em>'.
   * @generated
   */
  WMLArrayCall createWMLArrayCall();

  /**
   * Returns a new object of class '<em>WML Macro Define</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Macro Define</em>'.
   * @generated
   */
  WMLMacroDefine createWMLMacroDefine();

  /**
   * Returns a new object of class '<em>WML Preproc IF</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Preproc IF</em>'.
   * @generated
   */
  WMLPreprocIF createWMLPreprocIF();

  /**
   * Returns a new object of class '<em>WML Root Expression</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Root Expression</em>'.
   * @generated
   */
  WMLRootExpression createWMLRootExpression();

  /**
   * Returns a new object of class '<em>WML Expression</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Expression</em>'.
   * @generated
   */
  WMLExpression createWMLExpression();

  /**
   * Returns a new object of class '<em>WML Valued Expression</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Valued Expression</em>'.
   * @generated
   */
  WMLValuedExpression createWMLValuedExpression();

  /**
   * Returns a new object of class '<em>WML Textdomain</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Textdomain</em>'.
   * @generated
   */
  WMLTextdomain createWMLTextdomain();

  /**
   * Returns a new object of class '<em>WML Lua Code</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Lua Code</em>'.
   * @generated
   */
  WMLLuaCode createWMLLuaCode();

  /**
   * Returns a new object of class '<em>WML Macro Parameter</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Macro Parameter</em>'.
   * @generated
   */
  WMLMacroParameter createWMLMacroParameter();

  /**
   * Returns a new object of class '<em>WML Value</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>WML Value</em>'.
   * @generated
   */
  WMLValue createWMLValue();

  /**
   * Returns a new object of class '<em>Macro Tokens</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Macro Tokens</em>'.
   * @generated
   */
  MacroTokens createMacroTokens();

  /**
   * Returns the package supported by this factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the package supported by this factory.
   * @generated
   */
  WmlPackage getWmlPackage();

} //WmlFactory

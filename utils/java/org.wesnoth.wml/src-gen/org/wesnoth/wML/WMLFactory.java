/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EFactory;

/**
 * <!-- begin-user-doc -->
 * The <b>Factory</b> for the model.
 * It provides a create method for each non-abstract class of the model.
 * <!-- end-user-doc -->
 * @see org.wesnoth.wML.WMLPackage
 * @generated
 */
public interface WMLFactory extends EFactory
{
  /**
   * The singleton instance of the factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  WMLFactory eINSTANCE = org.wesnoth.wML.impl.WMLFactoryImpl.init();

  /**
   * Returns a new object of class '<em>Root</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Root</em>'.
   * @generated
   */
  Root createRoot();

  /**
   * Returns a new object of class '<em>Text Domain</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Text Domain</em>'.
   * @generated
   */
  TextDomain createTextDomain();

  /**
   * Returns a new object of class '<em>Preprocessor</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Preprocessor</em>'.
   * @generated
   */
  Preprocessor createPreprocessor();

  /**
   * Returns a new object of class '<em>Macro</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Macro</em>'.
   * @generated
   */
  Macro createMacro();

  /**
   * Returns a new object of class '<em>Path Include</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Path Include</em>'.
   * @generated
   */
  PathInclude createPathInclude();

  /**
   * Returns a new object of class '<em>Root Type</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Root Type</em>'.
   * @generated
   */
  RootType createRootType();

  /**
   * Returns a new object of class '<em>Root Tag</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Root Tag</em>'.
   * @generated
   */
  RootTag createRootTag();

  /**
   * Returns a new object of class '<em>Simple Tag</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Simple Tag</em>'.
   * @generated
   */
  SimpleTag createSimpleTag();

  /**
   * Returns a new object of class '<em>Added Tag</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Added Tag</em>'.
   * @generated
   */
  AddedTag createAddedTag();

  /**
   * Returns a new object of class '<em>Attributes</em>'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return a new object of class '<em>Attributes</em>'.
   * @generated
   */
  Attributes createAttributes();

  /**
   * Returns the package supported by this factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the package supported by this factory.
   * @generated
   */
  WMLPackage getWMLPackage();

} //WMLFactory

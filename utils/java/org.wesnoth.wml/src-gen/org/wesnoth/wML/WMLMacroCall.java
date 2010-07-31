/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Macro Call</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLMacroCall#isRelative <em>Relative</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroCall#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroCall#getParams <em>Params</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroCall#getExtraMacros <em>Extra Macros</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLMacroCall()
 * @model
 * @generated
 */
public interface WMLMacroCall extends WMLKeyValue
{
  /**
   * Returns the value of the '<em><b>Relative</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Relative</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Relative</em>' attribute.
   * @see #setRelative(boolean)
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroCall_Relative()
   * @model
   * @generated
   */
  boolean isRelative();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLMacroCall#isRelative <em>Relative</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Relative</em>' attribute.
   * @see #isRelative()
   * @generated
   */
  void setRelative(boolean value);

  /**
   * Returns the value of the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Name</em>' attribute.
   * @see #setName(String)
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroCall_Name()
   * @model
   * @generated
   */
  String getName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLMacroCall#getName <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Name</em>' attribute.
   * @see #getName()
   * @generated
   */
  void setName(String value);

  /**
   * Returns the value of the '<em><b>Params</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLValue}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Params</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Params</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroCall_Params()
   * @model containment="true"
   * @generated
   */
  EList<WMLValue> getParams();

  /**
   * Returns the value of the '<em><b>Extra Macros</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacroCall}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Extra Macros</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Extra Macros</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroCall_ExtraMacros()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroCall> getExtraMacros();

} // WMLMacroCall

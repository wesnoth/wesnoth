/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Root</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLRoot#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLRoot#getMacros <em>Macros</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLRoot#getMacrosDefines <em>Macros Defines</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLRoot()
 * @model
 * @generated
 */
public interface WMLRoot extends EObject
{
  /**
   * Returns the value of the '<em><b>Tags</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLTag}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Tags</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Tags</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLRoot_Tags()
   * @model containment="true"
   * @generated
   */
  EList<WMLTag> getTags();

  /**
   * Returns the value of the '<em><b>Macros</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLAbstractMacroCall}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macros</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macros</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLRoot_Macros()
   * @model containment="true"
   * @generated
   */
  EList<WMLAbstractMacroCall> getMacros();

  /**
   * Returns the value of the '<em><b>Macros Defines</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacroDefine}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macros Defines</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macros Defines</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLRoot_MacrosDefines()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroDefine> getMacrosDefines();

} // WMLRoot

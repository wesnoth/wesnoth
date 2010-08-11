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
 * A representation of the model object '<em><b>Macro Define</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLMacroDefine#getParams <em>Params</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroDefine#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroDefine#getMacros <em>Macros</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroDefine#getMacrosDefines <em>Macros Defines</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLMacroDefine#getKeys <em>Keys</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLMacroDefine()
 * @model
 * @generated
 */
public interface WMLMacroDefine extends EObject
{
  /**
   * Returns the value of the '<em><b>Params</b></em>' attribute list.
   * The list contents are of type {@link java.lang.String}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Params</em>' attribute list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Params</em>' attribute list.
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroDefine_Params()
   * @model unique="false"
   * @generated
   */
  EList<String> getParams();

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
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroDefine_Tags()
   * @model containment="true"
   * @generated
   */
  EList<WMLTag> getTags();

  /**
   * Returns the value of the '<em><b>Macros</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacroCall}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macros</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macros</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroDefine_Macros()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroCall> getMacros();

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
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroDefine_MacrosDefines()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroDefine> getMacrosDefines();

  /**
   * Returns the value of the '<em><b>Keys</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLKey}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Keys</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Keys</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLMacroDefine_Keys()
   * @model containment="true"
   * @generated
   */
  EList<WMLKey> getKeys();

} // WMLMacroDefine

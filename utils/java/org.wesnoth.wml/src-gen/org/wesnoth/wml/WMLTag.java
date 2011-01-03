/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Tag</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLTag#isPlus <em>Plus</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getKeys <em>Keys</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getMacroCalls <em>Macro Calls</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getMacroDefines <em>Macro Defines</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getTextdomains <em>Textdomains</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getIfDefs <em>If Defs</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLTag()
 * @model
 * @generated
 */
public interface WMLTag extends EObject
{
  /**
   * Returns the value of the '<em><b>Plus</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Plus</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Plus</em>' attribute.
   * @see #setPlus(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Plus()
   * @model
   * @generated
   */
  boolean isPlus();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#isPlus <em>Plus</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Plus</em>' attribute.
   * @see #isPlus()
   * @generated
   */
  void setPlus(boolean value);

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
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Name()
   * @model
   * @generated
   */
  String getName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#getName <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Name</em>' attribute.
   * @see #getName()
   * @generated
   */
  void setName(String value);

  /**
   * Returns the value of the '<em><b>Tags</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLTag}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Tags</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Tags</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Tags()
   * @model containment="true"
   * @generated
   */
  EList<WMLTag> getTags();

  /**
   * Returns the value of the '<em><b>Keys</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLKey}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Keys</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Keys</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Keys()
   * @model containment="true"
   * @generated
   */
  EList<WMLKey> getKeys();

  /**
   * Returns the value of the '<em><b>Macro Calls</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLMacroCall}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macro Calls</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macro Calls</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_MacroCalls()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroCall> getMacroCalls();

  /**
   * Returns the value of the '<em><b>Macro Defines</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLMacroDefine}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macro Defines</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macro Defines</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_MacroDefines()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroDefine> getMacroDefines();

  /**
   * Returns the value of the '<em><b>Textdomains</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLTextdomain}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Textdomains</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Textdomains</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Textdomains()
   * @model containment="true"
   * @generated
   */
  EList<WMLTextdomain> getTextdomains();

  /**
   * Returns the value of the '<em><b>If Defs</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLPreprocIF}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>If Defs</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>If Defs</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_IfDefs()
   * @model containment="true"
   * @generated
   */
  EList<WMLPreprocIF> getIfDefs();

  /**
   * Returns the value of the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End Name</em>' attribute.
   * @see #setEndName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_EndName()
   * @model
   * @generated
   */
  String getEndName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End Name</em>' attribute.
   * @see #getEndName()
   * @generated
   */
  void setEndName(String value);

} // WMLTag

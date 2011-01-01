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
 * A representation of the model object '<em><b>Preproc IF</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getKeys <em>Keys</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getMacroCalls <em>Macro Calls</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getMacroDefines <em>Macro Defines</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getTextdomains <em>Textdomains</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getValues <em>Values</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getIfDefs <em>If Defs</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getElses <em>Elses</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLPreprocIF#getEndName <em>End Name</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF()
 * @model
 * @generated
 */
public interface WMLPreprocIF extends EObject
{
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
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_Name()
   * @model
   * @generated
   */
  String getName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLPreprocIF#getName <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Name</em>' attribute.
   * @see #getName()
   * @generated
   */
  void setName(String value);

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
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_Tags()
   * @model containment="true"
   * @generated
   */
  EList<WMLTag> getTags();

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
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_Keys()
   * @model containment="true"
   * @generated
   */
  EList<WMLKey> getKeys();

  /**
   * Returns the value of the '<em><b>Macro Calls</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacroCall}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macro Calls</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macro Calls</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_MacroCalls()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroCall> getMacroCalls();

  /**
   * Returns the value of the '<em><b>Macro Defines</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacroDefine}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macro Defines</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macro Defines</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_MacroDefines()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroDefine> getMacroDefines();

  /**
   * Returns the value of the '<em><b>Textdomains</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLTextdomain}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Textdomains</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Textdomains</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_Textdomains()
   * @model containment="true"
   * @generated
   */
  EList<WMLTextdomain> getTextdomains();

  /**
   * Returns the value of the '<em><b>Values</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLValue}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Values</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Values</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_Values()
   * @model containment="true"
   * @generated
   */
  EList<WMLValue> getValues();

  /**
   * Returns the value of the '<em><b>If Defs</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLPreprocIF}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>If Defs</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>If Defs</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_IfDefs()
   * @model containment="true"
   * @generated
   */
  EList<WMLPreprocIF> getIfDefs();

  /**
   * Returns the value of the '<em><b>Elses</b></em>' attribute list.
   * The list contents are of type {@link java.lang.String}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Elses</em>' attribute list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Elses</em>' attribute list.
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_Elses()
   * @model unique="false"
   * @generated
   */
  EList<String> getElses();

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
   * @see org.wesnoth.wML.WMLPackage#getWMLPreprocIF_EndName()
   * @model
   * @generated
   */
  String getEndName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLPreprocIF#getEndName <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End Name</em>' attribute.
   * @see #getEndName()
   * @generated
   */
  void setEndName(String value);

} // WMLPreprocIF

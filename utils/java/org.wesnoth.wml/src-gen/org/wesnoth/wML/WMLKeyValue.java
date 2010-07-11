/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Key Value</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLKeyValue#getKey1Value <em>Key1 Value</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLKeyValue#getKey2Value <em>Key2 Value</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLKeyValue()
 * @model
 * @generated
 */
public interface WMLKeyValue extends EObject
{
  /**
   * Returns the value of the '<em><b>Key1 Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Key1 Value</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Key1 Value</em>' attribute.
   * @see #setKey1Value(String)
   * @see org.wesnoth.wML.WMLPackage#getWMLKeyValue_Key1Value()
   * @model
   * @generated
   */
  String getKey1Value();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLKeyValue#getKey1Value <em>Key1 Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Key1 Value</em>' attribute.
   * @see #getKey1Value()
   * @generated
   */
  void setKey1Value(String value);

  /**
   * Returns the value of the '<em><b>Key2 Value</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Key2 Value</em>' containment reference isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Key2 Value</em>' containment reference.
   * @see #setKey2Value(WMLMacro)
   * @see org.wesnoth.wML.WMLPackage#getWMLKeyValue_Key2Value()
   * @model containment="true"
   * @generated
   */
  WMLMacro getKey2Value();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLKeyValue#getKey2Value <em>Key2 Value</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Key2 Value</em>' containment reference.
   * @see #getKey2Value()
   * @generated
   */
  void setKey2Value(WMLMacro value);

} // WMLKeyValue

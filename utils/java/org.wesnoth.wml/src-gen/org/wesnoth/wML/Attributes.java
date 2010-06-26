/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Attributes</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.Attributes#getAttrName <em>Attr Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.Attributes#getAttrValue <em>Attr Value</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getAttributes()
 * @model
 * @generated
 */
public interface Attributes extends EObject
{
  /**
   * Returns the value of the '<em><b>Attr Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Attr Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Attr Name</em>' attribute.
   * @see #setAttrName(String)
   * @see org.wesnoth.wML.WMLPackage#getAttributes_AttrName()
   * @model
   * @generated
   */
  String getAttrName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.Attributes#getAttrName <em>Attr Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Attr Name</em>' attribute.
   * @see #getAttrName()
   * @generated
   */
  void setAttrName(String value);

  /**
   * Returns the value of the '<em><b>Attr Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Attr Value</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Attr Value</em>' attribute.
   * @see #setAttrValue(String)
   * @see org.wesnoth.wML.WMLPackage#getAttributes_AttrValue()
   * @model
   * @generated
   */
  String getAttrValue();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.Attributes#getAttrValue <em>Attr Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Attr Value</em>' attribute.
   * @see #getAttrValue()
   * @generated
   */
  void setAttrValue(String value);

} // Attributes

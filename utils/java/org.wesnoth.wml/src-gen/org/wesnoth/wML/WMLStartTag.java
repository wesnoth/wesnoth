/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Start Tag</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLStartTag#getTagname <em>Tagname</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLStartTag()
 * @model
 * @generated
 */
public interface WMLStartTag extends EObject
{
  /**
   * Returns the value of the '<em><b>Tagname</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Tagname</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Tagname</em>' attribute.
   * @see #setTagname(String)
   * @see org.wesnoth.wML.WMLPackage#getWMLStartTag_Tagname()
   * @model
   * @generated
   */
  String getTagname();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLStartTag#getTagname <em>Tagname</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Tagname</em>' attribute.
   * @see #getTagname()
   * @generated
   */
  void setTagname(String value);

} // WMLStartTag

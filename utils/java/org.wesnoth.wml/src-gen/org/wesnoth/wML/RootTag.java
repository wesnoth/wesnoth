/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Root Tag</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.RootTag#getTagName <em>Tag Name</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getRootTag()
 * @model
 * @generated
 */
public interface RootTag extends EObject
{
  /**
   * Returns the value of the '<em><b>Tag Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Tag Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Tag Name</em>' attribute.
   * @see #setTagName(String)
   * @see org.wesnoth.wML.WMLPackage#getRootTag_TagName()
   * @model
   * @generated
   */
  String getTagName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.RootTag#getTagName <em>Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Tag Name</em>' attribute.
   * @see #getTagName()
   * @generated
   */
  void setTagName(String value);

} // RootTag

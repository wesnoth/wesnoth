/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Simple Tag</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.SimpleTag#isEndTag <em>End Tag</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getSimpleTag()
 * @model
 * @generated
 */
public interface SimpleTag extends RootTag
{
  /**
   * Returns the value of the '<em><b>End Tag</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End Tag</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End Tag</em>' attribute.
   * @see #setEndTag(boolean)
   * @see org.wesnoth.wML.WMLPackage#getSimpleTag_EndTag()
   * @model
   * @generated
   */
  boolean isEndTag();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.SimpleTag#isEndTag <em>End Tag</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End Tag</em>' attribute.
   * @see #isEndTag()
   * @generated
   */
  void setEndTag(boolean value);

} // SimpleTag

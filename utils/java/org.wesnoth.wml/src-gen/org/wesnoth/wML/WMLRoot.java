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
 *   <li>{@link org.wesnoth.wML.WMLRoot#getRtags <em>Rtags</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLRoot#getRmacros <em>Rmacros</em>}</li>
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
   * Returns the value of the '<em><b>Rtags</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLTag}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Rtags</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Rtags</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLRoot_Rtags()
   * @model containment="true"
   * @generated
   */
  EList<WMLTag> getRtags();

  /**
   * Returns the value of the '<em><b>Rmacros</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacro}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Rmacros</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Rmacros</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLRoot_Rmacros()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacro> getRmacros();

} // WMLRoot

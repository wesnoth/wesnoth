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
 * A representation of the model object '<em><b>Tag</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLTag#getStart <em>Start</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLTag#getTtags <em>Ttags</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLTag#getTkeys <em>Tkeys</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLTag#getTmacros <em>Tmacros</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLTag#getEnd <em>End</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLTag()
 * @model
 * @generated
 */
public interface WMLTag extends EObject
{
  /**
   * Returns the value of the '<em><b>Start</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Start</em>' containment reference isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Start</em>' containment reference.
   * @see #setStart(WMLStartTag)
   * @see org.wesnoth.wML.WMLPackage#getWMLTag_Start()
   * @model containment="true"
   * @generated
   */
  WMLStartTag getStart();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLTag#getStart <em>Start</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Start</em>' containment reference.
   * @see #getStart()
   * @generated
   */
  void setStart(WMLStartTag value);

  /**
   * Returns the value of the '<em><b>Ttags</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLTag}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Ttags</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Ttags</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLTag_Ttags()
   * @model containment="true"
   * @generated
   */
  EList<WMLTag> getTtags();

  /**
   * Returns the value of the '<em><b>Tkeys</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLKey}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Tkeys</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Tkeys</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLTag_Tkeys()
   * @model containment="true"
   * @generated
   */
  EList<WMLKey> getTkeys();

  /**
   * Returns the value of the '<em><b>Tmacros</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLMacro}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Tmacros</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Tmacros</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLTag_Tmacros()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacro> getTmacros();

  /**
   * Returns the value of the '<em><b>End</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End</em>' containment reference isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End</em>' containment reference.
   * @see #setEnd(WMLEndTag)
   * @see org.wesnoth.wML.WMLPackage#getWMLTag_End()
   * @model containment="true"
   * @generated
   */
  WMLEndTag getEnd();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLTag#getEnd <em>End</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End</em>' containment reference.
   * @see #getEnd()
   * @generated
   */
  void setEnd(WMLEndTag value);

} // WMLTag

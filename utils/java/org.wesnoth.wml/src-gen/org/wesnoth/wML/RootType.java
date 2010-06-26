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
 * A representation of the model object '<em><b>Root Type</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.RootType#getStartTag <em>Start Tag</em>}</li>
 *   <li>{@link org.wesnoth.wML.RootType#getSubTypes <em>Sub Types</em>}</li>
 *   <li>{@link org.wesnoth.wML.RootType#getAt <em>At</em>}</li>
 *   <li>{@link org.wesnoth.wML.RootType#getOkpreproc <em>Okpreproc</em>}</li>
 *   <li>{@link org.wesnoth.wML.RootType#getEndTag <em>End Tag</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getRootType()
 * @model
 * @generated
 */
public interface RootType extends EObject
{
  /**
   * Returns the value of the '<em><b>Start Tag</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Start Tag</em>' containment reference isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Start Tag</em>' containment reference.
   * @see #setStartTag(RootTag)
   * @see org.wesnoth.wML.WMLPackage#getRootType_StartTag()
   * @model containment="true"
   * @generated
   */
  RootTag getStartTag();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.RootType#getStartTag <em>Start Tag</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Start Tag</em>' containment reference.
   * @see #getStartTag()
   * @generated
   */
  void setStartTag(RootTag value);

  /**
   * Returns the value of the '<em><b>Sub Types</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.RootType}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Sub Types</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Sub Types</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getRootType_SubTypes()
   * @model containment="true"
   * @generated
   */
  EList<RootType> getSubTypes();

  /**
   * Returns the value of the '<em><b>At</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.Attributes}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>At</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>At</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getRootType_At()
   * @model containment="true"
   * @generated
   */
  EList<Attributes> getAt();

  /**
   * Returns the value of the '<em><b>Okpreproc</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.Preprocessor}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Okpreproc</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Okpreproc</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getRootType_Okpreproc()
   * @model containment="true"
   * @generated
   */
  EList<Preprocessor> getOkpreproc();

  /**
   * Returns the value of the '<em><b>End Tag</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End Tag</em>' containment reference isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End Tag</em>' containment reference.
   * @see #setEndTag(RootTag)
   * @see org.wesnoth.wML.WMLPackage#getRootType_EndTag()
   * @model containment="true"
   * @generated
   */
  RootTag getEndTag();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.RootType#getEndTag <em>End Tag</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End Tag</em>' containment reference.
   * @see #getEndTag()
   * @generated
   */
  void setEndTag(RootTag value);

} // RootType

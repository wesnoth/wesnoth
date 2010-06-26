/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Text Domain</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.TextDomain#getDomainName <em>Domain Name</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getTextDomain()
 * @model
 * @generated
 */
public interface TextDomain extends EObject
{
  /**
   * Returns the value of the '<em><b>Domain Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Domain Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Domain Name</em>' attribute.
   * @see #setDomainName(String)
   * @see org.wesnoth.wML.WMLPackage#getTextDomain_DomainName()
   * @model
   * @generated
   */
  String getDomainName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.TextDomain#getDomainName <em>Domain Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Domain Name</em>' attribute.
   * @see #getDomainName()
   * @generated
   */
  void setDomainName(String value);

} // TextDomain

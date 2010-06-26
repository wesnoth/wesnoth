/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.common.notify.Notification;

import org.eclipse.emf.ecore.EClass;

import org.eclipse.emf.ecore.impl.ENotificationImpl;
import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.wesnoth.wML.TextDomain;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Text Domain</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.TextDomainImpl#getDomainName <em>Domain Name</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class TextDomainImpl extends MinimalEObjectImpl.Container implements TextDomain
{
  /**
   * The default value of the '{@link #getDomainName() <em>Domain Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getDomainName()
   * @generated
   * @ordered
   */
  protected static final String DOMAIN_NAME_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getDomainName() <em>Domain Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getDomainName()
   * @generated
   * @ordered
   */
  protected String domainName = DOMAIN_NAME_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected TextDomainImpl()
  {
    super();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  protected EClass eStaticClass()
  {
    return WMLPackage.Literals.TEXT_DOMAIN;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getDomainName()
  {
    return domainName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setDomainName(String newDomainName)
  {
    String oldDomainName = domainName;
    domainName = newDomainName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.TEXT_DOMAIN__DOMAIN_NAME, oldDomainName, domainName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public Object eGet(int featureID, boolean resolve, boolean coreType)
  {
    switch (featureID)
    {
      case WMLPackage.TEXT_DOMAIN__DOMAIN_NAME:
        return getDomainName();
    }
    return super.eGet(featureID, resolve, coreType);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public void eSet(int featureID, Object newValue)
  {
    switch (featureID)
    {
      case WMLPackage.TEXT_DOMAIN__DOMAIN_NAME:
        setDomainName((String)newValue);
        return;
    }
    super.eSet(featureID, newValue);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public void eUnset(int featureID)
  {
    switch (featureID)
    {
      case WMLPackage.TEXT_DOMAIN__DOMAIN_NAME:
        setDomainName(DOMAIN_NAME_EDEFAULT);
        return;
    }
    super.eUnset(featureID);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public boolean eIsSet(int featureID)
  {
    switch (featureID)
    {
      case WMLPackage.TEXT_DOMAIN__DOMAIN_NAME:
        return DOMAIN_NAME_EDEFAULT == null ? domainName != null : !DOMAIN_NAME_EDEFAULT.equals(domainName);
    }
    return super.eIsSet(featureID);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public String toString()
  {
    if (eIsProxy()) return super.toString();

    StringBuffer result = new StringBuffer(super.toString());
    result.append(" (DomainName: ");
    result.append(domainName);
    result.append(')');
    return result.toString();
  }

} //TextDomainImpl

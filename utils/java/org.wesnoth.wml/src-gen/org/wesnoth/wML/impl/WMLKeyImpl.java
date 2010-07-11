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

import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Key</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyImpl#getKeyName <em>Key Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyImpl#getKeyValue <em>Key Value</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLKeyImpl extends MinimalEObjectImpl.Container implements WMLKey
{
  /**
   * The default value of the '{@link #getKeyName() <em>Key Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKeyName()
   * @generated
   * @ordered
   */
  protected static final String KEY_NAME_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getKeyName() <em>Key Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKeyName()
   * @generated
   * @ordered
   */
  protected String keyName = KEY_NAME_EDEFAULT;

  /**
   * The default value of the '{@link #getKeyValue() <em>Key Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKeyValue()
   * @generated
   * @ordered
   */
  protected static final String KEY_VALUE_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getKeyValue() <em>Key Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKeyValue()
   * @generated
   * @ordered
   */
  protected String keyValue = KEY_VALUE_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLKeyImpl()
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
    return WMLPackage.Literals.WML_KEY;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getKeyName()
  {
    return keyName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setKeyName(String newKeyName)
  {
    String oldKeyName = keyName;
    keyName = newKeyName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY__KEY_NAME, oldKeyName, keyName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getKeyValue()
  {
    return keyValue;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setKeyValue(String newKeyValue)
  {
    String oldKeyValue = keyValue;
    keyValue = newKeyValue;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY__KEY_VALUE, oldKeyValue, keyValue));
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
      case WMLPackage.WML_KEY__KEY_NAME:
        return getKeyName();
      case WMLPackage.WML_KEY__KEY_VALUE:
        return getKeyValue();
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
      case WMLPackage.WML_KEY__KEY_NAME:
        setKeyName((String)newValue);
        return;
      case WMLPackage.WML_KEY__KEY_VALUE:
        setKeyValue((String)newValue);
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
      case WMLPackage.WML_KEY__KEY_NAME:
        setKeyName(KEY_NAME_EDEFAULT);
        return;
      case WMLPackage.WML_KEY__KEY_VALUE:
        setKeyValue(KEY_VALUE_EDEFAULT);
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
      case WMLPackage.WML_KEY__KEY_NAME:
        return KEY_NAME_EDEFAULT == null ? keyName != null : !KEY_NAME_EDEFAULT.equals(keyName);
      case WMLPackage.WML_KEY__KEY_VALUE:
        return KEY_VALUE_EDEFAULT == null ? keyValue != null : !KEY_VALUE_EDEFAULT.equals(keyValue);
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
    result.append(" (keyName: ");
    result.append(keyName);
    result.append(", keyValue: ");
    result.append(keyValue);
    result.append(')');
    return result.toString();
  }

} //WMLKeyImpl

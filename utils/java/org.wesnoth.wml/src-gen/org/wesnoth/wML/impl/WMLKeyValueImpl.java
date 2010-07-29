/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;
import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.wesnoth.wML.WMLKeyValue;
import org.wesnoth.wML.WMLMacro;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Key Value</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyValueImpl#getKey1Value <em>Key1 Value</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyValueImpl#getKey2Value <em>Key2 Value</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLKeyValueImpl extends MinimalEObjectImpl.Container implements WMLKeyValue
{
  /**
   * The default value of the '{@link #getKey1Value() <em>Key1 Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKey1Value()
   * @generated
   * @ordered
   */
  protected static final String KEY1_VALUE_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getKey1Value() <em>Key1 Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKey1Value()
   * @generated
   * @ordered
   */
  protected String key1Value = KEY1_VALUE_EDEFAULT;

  /**
   * The cached value of the '{@link #getKey2Value() <em>Key2 Value</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKey2Value()
   * @generated
   * @ordered
   */
  protected WMLMacro key2Value;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLKeyValueImpl()
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
    return WMLPackage.Literals.WML_KEY_VALUE;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getKey1Value()
  {
    return key1Value;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setKey1Value(String newKey1Value)
  {
    String oldKey1Value = key1Value;
    key1Value = newKey1Value;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY_VALUE__KEY1_VALUE, oldKey1Value, key1Value));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLMacro getKey2Value()
  {
    return key2Value;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public NotificationChain basicSetKey2Value(WMLMacro newKey2Value, NotificationChain msgs)
  {
    WMLMacro oldKey2Value = key2Value;
    key2Value = newKey2Value;
    if (eNotificationRequired())
    {
      ENotificationImpl notification = new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY_VALUE__KEY2_VALUE, oldKey2Value, newKey2Value);
      if (msgs == null) msgs = notification; else msgs.add(notification);
    }
    return msgs;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setKey2Value(WMLMacro newKey2Value)
  {
    if (newKey2Value != key2Value)
    {
      NotificationChain msgs = null;
      if (key2Value != null)
        msgs = ((InternalEObject)key2Value).eInverseRemove(this, EOPPOSITE_FEATURE_BASE - WMLPackage.WML_KEY_VALUE__KEY2_VALUE, null, msgs);
      if (newKey2Value != null)
        msgs = ((InternalEObject)newKey2Value).eInverseAdd(this, EOPPOSITE_FEATURE_BASE - WMLPackage.WML_KEY_VALUE__KEY2_VALUE, null, msgs);
      msgs = basicSetKey2Value(newKey2Value, msgs);
      if (msgs != null) msgs.dispatch();
    }
    else if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY_VALUE__KEY2_VALUE, newKey2Value, newKey2Value));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public NotificationChain eInverseRemove(InternalEObject otherEnd, int featureID, NotificationChain msgs)
  {
    switch (featureID)
    {
      case WMLPackage.WML_KEY_VALUE__KEY2_VALUE:
        return basicSetKey2Value(null, msgs);
    }
    return super.eInverseRemove(otherEnd, featureID, msgs);
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
      case WMLPackage.WML_KEY_VALUE__KEY1_VALUE:
        return getKey1Value();
      case WMLPackage.WML_KEY_VALUE__KEY2_VALUE:
        return getKey2Value();
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
      case WMLPackage.WML_KEY_VALUE__KEY1_VALUE:
        setKey1Value((String)newValue);
        return;
      case WMLPackage.WML_KEY_VALUE__KEY2_VALUE:
        setKey2Value((WMLMacro)newValue);
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
      case WMLPackage.WML_KEY_VALUE__KEY1_VALUE:
        setKey1Value(KEY1_VALUE_EDEFAULT);
        return;
      case WMLPackage.WML_KEY_VALUE__KEY2_VALUE:
        setKey2Value((WMLMacro)null);
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
      case WMLPackage.WML_KEY_VALUE__KEY1_VALUE:
        return KEY1_VALUE_EDEFAULT == null ? key1Value != null : !KEY1_VALUE_EDEFAULT.equals(key1Value);
      case WMLPackage.WML_KEY_VALUE__KEY2_VALUE:
        return key2Value != null;
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
    result.append(" (key1Value: ");
    result.append(key1Value);
    result.append(')');
    return result.toString();
  }

} //WMLKeyValueImpl

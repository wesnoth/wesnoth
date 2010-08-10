/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;
import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLKeyExtraArgs;
import org.wesnoth.wML.WMLKeyValue;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Key</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyImpl#getValue <em>Value</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLKeyImpl#getExtraArgs <em>Extra Args</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLKeyImpl extends MinimalEObjectImpl.Container implements WMLKey
{
  /**
   * The default value of the '{@link #getName() <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getName()
   * @generated
   * @ordered
   */
  protected static final String NAME_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getName() <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getName()
   * @generated
   * @ordered
   */
  protected String name = NAME_EDEFAULT;

  /**
   * The cached value of the '{@link #getValue() <em>Value</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getValue()
   * @generated
   * @ordered
   */
  protected WMLKeyValue value;

  /**
   * The cached value of the '{@link #getExtraArgs() <em>Extra Args</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getExtraArgs()
   * @generated
   * @ordered
   */
  protected EList<WMLKeyExtraArgs> extraArgs;

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
  public String getName()
  {
    return name;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setName(String newName)
  {
    String oldName = name;
    name = newName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY__NAME, oldName, name));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLKeyValue getValue()
  {
    return value;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public NotificationChain basicSetValue(WMLKeyValue newValue, NotificationChain msgs)
  {
    WMLKeyValue oldValue = value;
    value = newValue;
    if (eNotificationRequired())
    {
      ENotificationImpl notification = new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY__VALUE, oldValue, newValue);
      if (msgs == null) msgs = notification; else msgs.add(notification);
    }
    return msgs;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setValue(WMLKeyValue newValue)
  {
    if (newValue != value)
    {
      NotificationChain msgs = null;
      if (value != null)
        msgs = ((InternalEObject)value).eInverseRemove(this, EOPPOSITE_FEATURE_BASE - WMLPackage.WML_KEY__VALUE, null, msgs);
      if (newValue != null)
        msgs = ((InternalEObject)newValue).eInverseAdd(this, EOPPOSITE_FEATURE_BASE - WMLPackage.WML_KEY__VALUE, null, msgs);
      msgs = basicSetValue(newValue, msgs);
      if (msgs != null) msgs.dispatch();
    }
    else if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_KEY__VALUE, newValue, newValue));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLKeyExtraArgs> getExtraArgs()
  {
    if (extraArgs == null)
    {
      extraArgs = new EObjectContainmentEList<WMLKeyExtraArgs>(WMLKeyExtraArgs.class, this, WMLPackage.WML_KEY__EXTRA_ARGS);
    }
    return extraArgs;
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
      case WMLPackage.WML_KEY__VALUE:
        return basicSetValue(null, msgs);
      case WMLPackage.WML_KEY__EXTRA_ARGS:
        return ((InternalEList<?>)getExtraArgs()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_KEY__NAME:
        return getName();
      case WMLPackage.WML_KEY__VALUE:
        return getValue();
      case WMLPackage.WML_KEY__EXTRA_ARGS:
        return getExtraArgs();
    }
    return super.eGet(featureID, resolve, coreType);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @SuppressWarnings("unchecked")
  @Override
  public void eSet(int featureID, Object newValue)
  {
    switch (featureID)
    {
      case WMLPackage.WML_KEY__NAME:
        setName((String)newValue);
        return;
      case WMLPackage.WML_KEY__VALUE:
        setValue((WMLKeyValue)newValue);
        return;
      case WMLPackage.WML_KEY__EXTRA_ARGS:
        getExtraArgs().clear();
        getExtraArgs().addAll((Collection<? extends WMLKeyExtraArgs>)newValue);
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
      case WMLPackage.WML_KEY__NAME:
        setName(NAME_EDEFAULT);
        return;
      case WMLPackage.WML_KEY__VALUE:
        setValue((WMLKeyValue)null);
        return;
      case WMLPackage.WML_KEY__EXTRA_ARGS:
        getExtraArgs().clear();
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
      case WMLPackage.WML_KEY__NAME:
        return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
      case WMLPackage.WML_KEY__VALUE:
        return value != null;
      case WMLPackage.WML_KEY__EXTRA_ARGS:
        return extraArgs != null && !extraArgs.isEmpty();
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
    result.append(" (name: ");
    result.append(name);
    result.append(')');
    return result.toString();
  }

} //WMLKeyImpl

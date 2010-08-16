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
import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLMacroDefine;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLPreprocIF;
import org.wesnoth.wML.WMLTag;
import org.wesnoth.wML.WMLTextdomain;
import org.wesnoth.wML.WMLValue;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Macro Define</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getKeys <em>Keys</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getMacroCalls <em>Macro Calls</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getMacroDefines <em>Macro Defines</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getTextdomains <em>Textdomains</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getValues <em>Values</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroDefineImpl#getIfDefs <em>If Defs</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLMacroDefineImpl extends MinimalEObjectImpl.Container implements WMLMacroDefine
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
   * The cached value of the '{@link #getTags() <em>Tags</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTags()
   * @generated
   * @ordered
   */
  protected EList<WMLTag> tags;

  /**
   * The cached value of the '{@link #getKeys() <em>Keys</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getKeys()
   * @generated
   * @ordered
   */
  protected EList<WMLKey> keys;

  /**
   * The cached value of the '{@link #getMacroCalls() <em>Macro Calls</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getMacroCalls()
   * @generated
   * @ordered
   */
  protected EList<WMLMacroCall> macroCalls;

  /**
   * The cached value of the '{@link #getMacroDefines() <em>Macro Defines</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getMacroDefines()
   * @generated
   * @ordered
   */
  protected EList<WMLMacroDefine> macroDefines;

  /**
   * The cached value of the '{@link #getTextdomains() <em>Textdomains</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTextdomains()
   * @generated
   * @ordered
   */
  protected EList<WMLTextdomain> textdomains;

  /**
   * The cached value of the '{@link #getValues() <em>Values</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getValues()
   * @generated
   * @ordered
   */
  protected EList<WMLValue> values;

  /**
   * The cached value of the '{@link #getIfDefs() <em>If Defs</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getIfDefs()
   * @generated
   * @ordered
   */
  protected EList<WMLPreprocIF> ifDefs;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLMacroDefineImpl()
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
    return WMLPackage.Literals.WML_MACRO_DEFINE;
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
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_MACRO_DEFINE__NAME, oldName, name));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLTag> getTags()
  {
    if (tags == null)
    {
      tags = new EObjectContainmentEList<WMLTag>(WMLTag.class, this, WMLPackage.WML_MACRO_DEFINE__TAGS);
    }
    return tags;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLKey> getKeys()
  {
    if (keys == null)
    {
      keys = new EObjectContainmentEList<WMLKey>(WMLKey.class, this, WMLPackage.WML_MACRO_DEFINE__KEYS);
    }
    return keys;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacroCall> getMacroCalls()
  {
    if (macroCalls == null)
    {
      macroCalls = new EObjectContainmentEList<WMLMacroCall>(WMLMacroCall.class, this, WMLPackage.WML_MACRO_DEFINE__MACRO_CALLS);
    }
    return macroCalls;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacroDefine> getMacroDefines()
  {
    if (macroDefines == null)
    {
      macroDefines = new EObjectContainmentEList<WMLMacroDefine>(WMLMacroDefine.class, this, WMLPackage.WML_MACRO_DEFINE__MACRO_DEFINES);
    }
    return macroDefines;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLTextdomain> getTextdomains()
  {
    if (textdomains == null)
    {
      textdomains = new EObjectContainmentEList<WMLTextdomain>(WMLTextdomain.class, this, WMLPackage.WML_MACRO_DEFINE__TEXTDOMAINS);
    }
    return textdomains;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLValue> getValues()
  {
    if (values == null)
    {
      values = new EObjectContainmentEList<WMLValue>(WMLValue.class, this, WMLPackage.WML_MACRO_DEFINE__VALUES);
    }
    return values;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLPreprocIF> getIfDefs()
  {
    if (ifDefs == null)
    {
      ifDefs = new EObjectContainmentEList<WMLPreprocIF>(WMLPreprocIF.class, this, WMLPackage.WML_MACRO_DEFINE__IF_DEFS);
    }
    return ifDefs;
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
      case WMLPackage.WML_MACRO_DEFINE__TAGS:
        return ((InternalEList<?>)getTags()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_DEFINE__KEYS:
        return ((InternalEList<?>)getKeys()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_DEFINE__MACRO_CALLS:
        return ((InternalEList<?>)getMacroCalls()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_DEFINE__MACRO_DEFINES:
        return ((InternalEList<?>)getMacroDefines()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_DEFINE__TEXTDOMAINS:
        return ((InternalEList<?>)getTextdomains()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_DEFINE__VALUES:
        return ((InternalEList<?>)getValues()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_DEFINE__IF_DEFS:
        return ((InternalEList<?>)getIfDefs()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_MACRO_DEFINE__NAME:
        return getName();
      case WMLPackage.WML_MACRO_DEFINE__TAGS:
        return getTags();
      case WMLPackage.WML_MACRO_DEFINE__KEYS:
        return getKeys();
      case WMLPackage.WML_MACRO_DEFINE__MACRO_CALLS:
        return getMacroCalls();
      case WMLPackage.WML_MACRO_DEFINE__MACRO_DEFINES:
        return getMacroDefines();
      case WMLPackage.WML_MACRO_DEFINE__TEXTDOMAINS:
        return getTextdomains();
      case WMLPackage.WML_MACRO_DEFINE__VALUES:
        return getValues();
      case WMLPackage.WML_MACRO_DEFINE__IF_DEFS:
        return getIfDefs();
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
      case WMLPackage.WML_MACRO_DEFINE__NAME:
        setName((String)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__TAGS:
        getTags().clear();
        getTags().addAll((Collection<? extends WMLTag>)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__KEYS:
        getKeys().clear();
        getKeys().addAll((Collection<? extends WMLKey>)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__MACRO_CALLS:
        getMacroCalls().clear();
        getMacroCalls().addAll((Collection<? extends WMLMacroCall>)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__MACRO_DEFINES:
        getMacroDefines().clear();
        getMacroDefines().addAll((Collection<? extends WMLMacroDefine>)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__TEXTDOMAINS:
        getTextdomains().clear();
        getTextdomains().addAll((Collection<? extends WMLTextdomain>)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__VALUES:
        getValues().clear();
        getValues().addAll((Collection<? extends WMLValue>)newValue);
        return;
      case WMLPackage.WML_MACRO_DEFINE__IF_DEFS:
        getIfDefs().clear();
        getIfDefs().addAll((Collection<? extends WMLPreprocIF>)newValue);
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
      case WMLPackage.WML_MACRO_DEFINE__NAME:
        setName(NAME_EDEFAULT);
        return;
      case WMLPackage.WML_MACRO_DEFINE__TAGS:
        getTags().clear();
        return;
      case WMLPackage.WML_MACRO_DEFINE__KEYS:
        getKeys().clear();
        return;
      case WMLPackage.WML_MACRO_DEFINE__MACRO_CALLS:
        getMacroCalls().clear();
        return;
      case WMLPackage.WML_MACRO_DEFINE__MACRO_DEFINES:
        getMacroDefines().clear();
        return;
      case WMLPackage.WML_MACRO_DEFINE__TEXTDOMAINS:
        getTextdomains().clear();
        return;
      case WMLPackage.WML_MACRO_DEFINE__VALUES:
        getValues().clear();
        return;
      case WMLPackage.WML_MACRO_DEFINE__IF_DEFS:
        getIfDefs().clear();
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
      case WMLPackage.WML_MACRO_DEFINE__NAME:
        return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
      case WMLPackage.WML_MACRO_DEFINE__TAGS:
        return tags != null && !tags.isEmpty();
      case WMLPackage.WML_MACRO_DEFINE__KEYS:
        return keys != null && !keys.isEmpty();
      case WMLPackage.WML_MACRO_DEFINE__MACRO_CALLS:
        return macroCalls != null && !macroCalls.isEmpty();
      case WMLPackage.WML_MACRO_DEFINE__MACRO_DEFINES:
        return macroDefines != null && !macroDefines.isEmpty();
      case WMLPackage.WML_MACRO_DEFINE__TEXTDOMAINS:
        return textdomains != null && !textdomains.isEmpty();
      case WMLPackage.WML_MACRO_DEFINE__VALUES:
        return values != null && !values.isEmpty();
      case WMLPackage.WML_MACRO_DEFINE__IF_DEFS:
        return ifDefs != null && !ifDefs.isEmpty();
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

} //WMLMacroDefineImpl

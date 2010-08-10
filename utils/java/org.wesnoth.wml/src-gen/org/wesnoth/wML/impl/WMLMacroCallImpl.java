/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.util.EDataTypeEList;
import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLMacroDefine;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLTag;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Macro Call</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getArgs <em>Args</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getParams <em>Params</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getMacros <em>Macros</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getMacrosDefines <em>Macros Defines</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getKeys <em>Keys</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLMacroCallImpl extends WMLAbstractMacroCallImpl implements WMLMacroCall
{
  /**
   * The cached value of the '{@link #getArgs() <em>Args</em>}' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getArgs()
   * @generated
   * @ordered
   */
  protected EList<String> args;

  /**
   * The cached value of the '{@link #getParams() <em>Params</em>}' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getParams()
   * @generated
   * @ordered
   */
  protected EList<String> params;

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
   * The cached value of the '{@link #getMacros() <em>Macros</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getMacros()
   * @generated
   * @ordered
   */
  protected EList<WMLMacroCall> macros;

  /**
   * The cached value of the '{@link #getMacrosDefines() <em>Macros Defines</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getMacrosDefines()
   * @generated
   * @ordered
   */
  protected EList<WMLMacroDefine> macrosDefines;

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
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLMacroCallImpl()
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
    return WMLPackage.Literals.WML_MACRO_CALL;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<String> getArgs()
  {
    if (args == null)
    {
      args = new EDataTypeEList<String>(String.class, this, WMLPackage.WML_MACRO_CALL__ARGS);
    }
    return args;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<String> getParams()
  {
    if (params == null)
    {
      params = new EDataTypeEList<String>(String.class, this, WMLPackage.WML_MACRO_CALL__PARAMS);
    }
    return params;
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
      tags = new EObjectContainmentEList<WMLTag>(WMLTag.class, this, WMLPackage.WML_MACRO_CALL__TAGS);
    }
    return tags;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacroCall> getMacros()
  {
    if (macros == null)
    {
      macros = new EObjectContainmentEList<WMLMacroCall>(WMLMacroCall.class, this, WMLPackage.WML_MACRO_CALL__MACROS);
    }
    return macros;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacroDefine> getMacrosDefines()
  {
    if (macrosDefines == null)
    {
      macrosDefines = new EObjectContainmentEList<WMLMacroDefine>(WMLMacroDefine.class, this, WMLPackage.WML_MACRO_CALL__MACROS_DEFINES);
    }
    return macrosDefines;
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
      keys = new EObjectContainmentEList<WMLKey>(WMLKey.class, this, WMLPackage.WML_MACRO_CALL__KEYS);
    }
    return keys;
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
      case WMLPackage.WML_MACRO_CALL__TAGS:
        return ((InternalEList<?>)getTags()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_CALL__MACROS:
        return ((InternalEList<?>)getMacros()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_CALL__MACROS_DEFINES:
        return ((InternalEList<?>)getMacrosDefines()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_CALL__KEYS:
        return ((InternalEList<?>)getKeys()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_MACRO_CALL__ARGS:
        return getArgs();
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        return getParams();
      case WMLPackage.WML_MACRO_CALL__TAGS:
        return getTags();
      case WMLPackage.WML_MACRO_CALL__MACROS:
        return getMacros();
      case WMLPackage.WML_MACRO_CALL__MACROS_DEFINES:
        return getMacrosDefines();
      case WMLPackage.WML_MACRO_CALL__KEYS:
        return getKeys();
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
      case WMLPackage.WML_MACRO_CALL__ARGS:
        getArgs().clear();
        getArgs().addAll((Collection<? extends String>)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        getParams().clear();
        getParams().addAll((Collection<? extends String>)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__TAGS:
        getTags().clear();
        getTags().addAll((Collection<? extends WMLTag>)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__MACROS:
        getMacros().clear();
        getMacros().addAll((Collection<? extends WMLMacroCall>)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__MACROS_DEFINES:
        getMacrosDefines().clear();
        getMacrosDefines().addAll((Collection<? extends WMLMacroDefine>)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__KEYS:
        getKeys().clear();
        getKeys().addAll((Collection<? extends WMLKey>)newValue);
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
      case WMLPackage.WML_MACRO_CALL__ARGS:
        getArgs().clear();
        return;
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        getParams().clear();
        return;
      case WMLPackage.WML_MACRO_CALL__TAGS:
        getTags().clear();
        return;
      case WMLPackage.WML_MACRO_CALL__MACROS:
        getMacros().clear();
        return;
      case WMLPackage.WML_MACRO_CALL__MACROS_DEFINES:
        getMacrosDefines().clear();
        return;
      case WMLPackage.WML_MACRO_CALL__KEYS:
        getKeys().clear();
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
      case WMLPackage.WML_MACRO_CALL__ARGS:
        return args != null && !args.isEmpty();
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        return params != null && !params.isEmpty();
      case WMLPackage.WML_MACRO_CALL__TAGS:
        return tags != null && !tags.isEmpty();
      case WMLPackage.WML_MACRO_CALL__MACROS:
        return macros != null && !macros.isEmpty();
      case WMLPackage.WML_MACRO_CALL__MACROS_DEFINES:
        return macrosDefines != null && !macrosDefines.isEmpty();
      case WMLPackage.WML_MACRO_CALL__KEYS:
        return keys != null && !keys.isEmpty();
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
    result.append(" (args: ");
    result.append(args);
    result.append(", params: ");
    result.append(params);
    result.append(')');
    return result.toString();
  }

} //WMLMacroCallImpl

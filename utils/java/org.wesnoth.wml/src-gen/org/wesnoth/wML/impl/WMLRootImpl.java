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

import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wML.WMLAbstractMacroCall;
import org.wesnoth.wML.WMLMacroDefine;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Root</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getMacros <em>Macros</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getMacrosDefines <em>Macros Defines</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLRootImpl extends MinimalEObjectImpl.Container implements WMLRoot
{
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
  protected EList<WMLAbstractMacroCall> macros;

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
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLRootImpl()
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
    return WMLPackage.Literals.WML_ROOT;
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
      tags = new EObjectContainmentEList<WMLTag>(WMLTag.class, this, WMLPackage.WML_ROOT__TAGS);
    }
    return tags;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLAbstractMacroCall> getMacros()
  {
    if (macros == null)
    {
      macros = new EObjectContainmentEList<WMLAbstractMacroCall>(WMLAbstractMacroCall.class, this, WMLPackage.WML_ROOT__MACROS);
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
      macrosDefines = new EObjectContainmentEList<WMLMacroDefine>(WMLMacroDefine.class, this, WMLPackage.WML_ROOT__MACROS_DEFINES);
    }
    return macrosDefines;
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
      case WMLPackage.WML_ROOT__TAGS:
        return ((InternalEList<?>)getTags()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_ROOT__MACROS:
        return ((InternalEList<?>)getMacros()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_ROOT__MACROS_DEFINES:
        return ((InternalEList<?>)getMacrosDefines()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_ROOT__TAGS:
        return getTags();
      case WMLPackage.WML_ROOT__MACROS:
        return getMacros();
      case WMLPackage.WML_ROOT__MACROS_DEFINES:
        return getMacrosDefines();
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
      case WMLPackage.WML_ROOT__TAGS:
        getTags().clear();
        getTags().addAll((Collection<? extends WMLTag>)newValue);
        return;
      case WMLPackage.WML_ROOT__MACROS:
        getMacros().clear();
        getMacros().addAll((Collection<? extends WMLAbstractMacroCall>)newValue);
        return;
      case WMLPackage.WML_ROOT__MACROS_DEFINES:
        getMacrosDefines().clear();
        getMacrosDefines().addAll((Collection<? extends WMLMacroDefine>)newValue);
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
      case WMLPackage.WML_ROOT__TAGS:
        getTags().clear();
        return;
      case WMLPackage.WML_ROOT__MACROS:
        getMacros().clear();
        return;
      case WMLPackage.WML_ROOT__MACROS_DEFINES:
        getMacrosDefines().clear();
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
      case WMLPackage.WML_ROOT__TAGS:
        return tags != null && !tags.isEmpty();
      case WMLPackage.WML_ROOT__MACROS:
        return macros != null && !macros.isEmpty();
      case WMLPackage.WML_ROOT__MACROS_DEFINES:
        return macrosDefines != null && !macrosDefines.isEmpty();
    }
    return super.eIsSet(featureID);
  }

} //WMLRootImpl

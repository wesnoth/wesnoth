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

import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLMacroDefine;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;
import org.wesnoth.wML.WMLTextdomain;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Root</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getMacroCalls <em>Macro Calls</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getMacroDefines <em>Macro Defines</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getTextdomains <em>Textdomains</em>}</li>
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
  public EList<WMLMacroCall> getMacroCalls()
  {
    if (macroCalls == null)
    {
      macroCalls = new EObjectContainmentEList<WMLMacroCall>(WMLMacroCall.class, this, WMLPackage.WML_ROOT__MACRO_CALLS);
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
      macroDefines = new EObjectContainmentEList<WMLMacroDefine>(WMLMacroDefine.class, this, WMLPackage.WML_ROOT__MACRO_DEFINES);
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
      textdomains = new EObjectContainmentEList<WMLTextdomain>(WMLTextdomain.class, this, WMLPackage.WML_ROOT__TEXTDOMAINS);
    }
    return textdomains;
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
      case WMLPackage.WML_ROOT__MACRO_CALLS:
        return ((InternalEList<?>)getMacroCalls()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_ROOT__MACRO_DEFINES:
        return ((InternalEList<?>)getMacroDefines()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_ROOT__TEXTDOMAINS:
        return ((InternalEList<?>)getTextdomains()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_ROOT__MACRO_CALLS:
        return getMacroCalls();
      case WMLPackage.WML_ROOT__MACRO_DEFINES:
        return getMacroDefines();
      case WMLPackage.WML_ROOT__TEXTDOMAINS:
        return getTextdomains();
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
      case WMLPackage.WML_ROOT__MACRO_CALLS:
        getMacroCalls().clear();
        getMacroCalls().addAll((Collection<? extends WMLMacroCall>)newValue);
        return;
      case WMLPackage.WML_ROOT__MACRO_DEFINES:
        getMacroDefines().clear();
        getMacroDefines().addAll((Collection<? extends WMLMacroDefine>)newValue);
        return;
      case WMLPackage.WML_ROOT__TEXTDOMAINS:
        getTextdomains().clear();
        getTextdomains().addAll((Collection<? extends WMLTextdomain>)newValue);
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
      case WMLPackage.WML_ROOT__MACRO_CALLS:
        getMacroCalls().clear();
        return;
      case WMLPackage.WML_ROOT__MACRO_DEFINES:
        getMacroDefines().clear();
        return;
      case WMLPackage.WML_ROOT__TEXTDOMAINS:
        getTextdomains().clear();
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
      case WMLPackage.WML_ROOT__MACRO_CALLS:
        return macroCalls != null && !macroCalls.isEmpty();
      case WMLPackage.WML_ROOT__MACRO_DEFINES:
        return macroDefines != null && !macroDefines.isEmpty();
      case WMLPackage.WML_ROOT__TEXTDOMAINS:
        return textdomains != null && !textdomains.isEmpty();
    }
    return super.eIsSet(featureID);
  }

} //WMLRootImpl

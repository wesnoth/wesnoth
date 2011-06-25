/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLMacroDefine;
import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLRoot;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLTextdomain;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>WML Root</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wml.impl.WMLRootImpl#getTags <em>Tags</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLRootImpl#getMacroCalls <em>Macro Calls</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLRootImpl#getMacroDefines <em>Macro Defines</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLRootImpl#getTextdomains <em>Textdomains</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLRootImpl#getIfDefs <em>If Defs</em>}</li>
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
    return WmlPackage.Literals.WML_ROOT;
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
      tags = new EObjectContainmentEList<WMLTag>(WMLTag.class, this, WmlPackage.WML_ROOT__TAGS);
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
      macroCalls = new EObjectContainmentEList<WMLMacroCall>(WMLMacroCall.class, this, WmlPackage.WML_ROOT__MACRO_CALLS);
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
      macroDefines = new EObjectContainmentEList<WMLMacroDefine>(WMLMacroDefine.class, this, WmlPackage.WML_ROOT__MACRO_DEFINES);
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
      textdomains = new EObjectContainmentEList<WMLTextdomain>(WMLTextdomain.class, this, WmlPackage.WML_ROOT__TEXTDOMAINS);
    }
    return textdomains;
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
      ifDefs = new EObjectContainmentEList<WMLPreprocIF>(WMLPreprocIF.class, this, WmlPackage.WML_ROOT__IF_DEFS);
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
      case WmlPackage.WML_ROOT__TAGS:
        return ((InternalEList<?>)getTags()).basicRemove(otherEnd, msgs);
      case WmlPackage.WML_ROOT__MACRO_CALLS:
        return ((InternalEList<?>)getMacroCalls()).basicRemove(otherEnd, msgs);
      case WmlPackage.WML_ROOT__MACRO_DEFINES:
        return ((InternalEList<?>)getMacroDefines()).basicRemove(otherEnd, msgs);
      case WmlPackage.WML_ROOT__TEXTDOMAINS:
        return ((InternalEList<?>)getTextdomains()).basicRemove(otherEnd, msgs);
      case WmlPackage.WML_ROOT__IF_DEFS:
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
      case WmlPackage.WML_ROOT__TAGS:
        return getTags();
      case WmlPackage.WML_ROOT__MACRO_CALLS:
        return getMacroCalls();
      case WmlPackage.WML_ROOT__MACRO_DEFINES:
        return getMacroDefines();
      case WmlPackage.WML_ROOT__TEXTDOMAINS:
        return getTextdomains();
      case WmlPackage.WML_ROOT__IF_DEFS:
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
      case WmlPackage.WML_ROOT__TAGS:
        getTags().clear();
        getTags().addAll((Collection<? extends WMLTag>)newValue);
        return;
      case WmlPackage.WML_ROOT__MACRO_CALLS:
        getMacroCalls().clear();
        getMacroCalls().addAll((Collection<? extends WMLMacroCall>)newValue);
        return;
      case WmlPackage.WML_ROOT__MACRO_DEFINES:
        getMacroDefines().clear();
        getMacroDefines().addAll((Collection<? extends WMLMacroDefine>)newValue);
        return;
      case WmlPackage.WML_ROOT__TEXTDOMAINS:
        getTextdomains().clear();
        getTextdomains().addAll((Collection<? extends WMLTextdomain>)newValue);
        return;
      case WmlPackage.WML_ROOT__IF_DEFS:
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
      case WmlPackage.WML_ROOT__TAGS:
        getTags().clear();
        return;
      case WmlPackage.WML_ROOT__MACRO_CALLS:
        getMacroCalls().clear();
        return;
      case WmlPackage.WML_ROOT__MACRO_DEFINES:
        getMacroDefines().clear();
        return;
      case WmlPackage.WML_ROOT__TEXTDOMAINS:
        getTextdomains().clear();
        return;
      case WmlPackage.WML_ROOT__IF_DEFS:
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
      case WmlPackage.WML_ROOT__TAGS:
        return tags != null && !tags.isEmpty();
      case WmlPackage.WML_ROOT__MACRO_CALLS:
        return macroCalls != null && !macroCalls.isEmpty();
      case WmlPackage.WML_ROOT__MACRO_DEFINES:
        return macroDefines != null && !macroDefines.isEmpty();
      case WmlPackage.WML_ROOT__TEXTDOMAINS:
        return textdomains != null && !textdomains.isEmpty();
      case WmlPackage.WML_ROOT__IF_DEFS:
        return ifDefs != null && !ifDefs.isEmpty();
    }
    return super.eIsSet(featureID);
  }

} //WMLRootImpl

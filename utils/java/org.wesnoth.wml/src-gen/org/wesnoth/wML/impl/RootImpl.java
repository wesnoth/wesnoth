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

import org.wesnoth.wML.Preprocessor;
import org.wesnoth.wML.Root;
import org.wesnoth.wML.RootType;
import org.wesnoth.wML.TextDomain;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Root</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.RootImpl#getTextdomains <em>Textdomains</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.RootImpl#getPreproc <em>Preproc</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.RootImpl#getRoots <em>Roots</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class RootImpl extends MinimalEObjectImpl.Container implements Root
{
  /**
   * The cached value of the '{@link #getTextdomains() <em>Textdomains</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTextdomains()
   * @generated
   * @ordered
   */
  protected EList<TextDomain> textdomains;

  /**
   * The cached value of the '{@link #getPreproc() <em>Preproc</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getPreproc()
   * @generated
   * @ordered
   */
  protected EList<Preprocessor> preproc;

  /**
   * The cached value of the '{@link #getRoots() <em>Roots</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getRoots()
   * @generated
   * @ordered
   */
  protected EList<RootType> roots;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected RootImpl()
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
    return WMLPackage.Literals.ROOT;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<TextDomain> getTextdomains()
  {
    if (textdomains == null)
    {
      textdomains = new EObjectContainmentEList<TextDomain>(TextDomain.class, this, WMLPackage.ROOT__TEXTDOMAINS);
    }
    return textdomains;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<Preprocessor> getPreproc()
  {
    if (preproc == null)
    {
      preproc = new EObjectContainmentEList<Preprocessor>(Preprocessor.class, this, WMLPackage.ROOT__PREPROC);
    }
    return preproc;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<RootType> getRoots()
  {
    if (roots == null)
    {
      roots = new EObjectContainmentEList<RootType>(RootType.class, this, WMLPackage.ROOT__ROOTS);
    }
    return roots;
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
      case WMLPackage.ROOT__TEXTDOMAINS:
        return ((InternalEList<?>)getTextdomains()).basicRemove(otherEnd, msgs);
      case WMLPackage.ROOT__PREPROC:
        return ((InternalEList<?>)getPreproc()).basicRemove(otherEnd, msgs);
      case WMLPackage.ROOT__ROOTS:
        return ((InternalEList<?>)getRoots()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.ROOT__TEXTDOMAINS:
        return getTextdomains();
      case WMLPackage.ROOT__PREPROC:
        return getPreproc();
      case WMLPackage.ROOT__ROOTS:
        return getRoots();
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
      case WMLPackage.ROOT__TEXTDOMAINS:
        getTextdomains().clear();
        getTextdomains().addAll((Collection<? extends TextDomain>)newValue);
        return;
      case WMLPackage.ROOT__PREPROC:
        getPreproc().clear();
        getPreproc().addAll((Collection<? extends Preprocessor>)newValue);
        return;
      case WMLPackage.ROOT__ROOTS:
        getRoots().clear();
        getRoots().addAll((Collection<? extends RootType>)newValue);
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
      case WMLPackage.ROOT__TEXTDOMAINS:
        getTextdomains().clear();
        return;
      case WMLPackage.ROOT__PREPROC:
        getPreproc().clear();
        return;
      case WMLPackage.ROOT__ROOTS:
        getRoots().clear();
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
      case WMLPackage.ROOT__TEXTDOMAINS:
        return textdomains != null && !textdomains.isEmpty();
      case WMLPackage.ROOT__PREPROC:
        return preproc != null && !preproc.isEmpty();
      case WMLPackage.ROOT__ROOTS:
        return roots != null && !roots.isEmpty();
    }
    return super.eIsSet(featureID);
  }

} //RootImpl

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

import org.wesnoth.wML.RootTag;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Root Tag</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.RootTagImpl#getTagName <em>Tag Name</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class RootTagImpl extends MinimalEObjectImpl.Container implements RootTag
{
  /**
   * The default value of the '{@link #getTagName() <em>Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTagName()
   * @generated
   * @ordered
   */
  protected static final String TAG_NAME_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getTagName() <em>Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTagName()
   * @generated
   * @ordered
   */
  protected String tagName = TAG_NAME_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected RootTagImpl()
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
    return WMLPackage.Literals.ROOT_TAG;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getTagName()
  {
    return tagName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setTagName(String newTagName)
  {
    String oldTagName = tagName;
    tagName = newTagName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.ROOT_TAG__TAG_NAME, oldTagName, tagName));
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
      case WMLPackage.ROOT_TAG__TAG_NAME:
        return getTagName();
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
      case WMLPackage.ROOT_TAG__TAG_NAME:
        setTagName((String)newValue);
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
      case WMLPackage.ROOT_TAG__TAG_NAME:
        setTagName(TAG_NAME_EDEFAULT);
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
      case WMLPackage.ROOT_TAG__TAG_NAME:
        return TAG_NAME_EDEFAULT == null ? tagName != null : !TAG_NAME_EDEFAULT.equals(tagName);
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
    result.append(" (tagName: ");
    result.append(tagName);
    result.append(')');
    return result.toString();
  }

} //RootTagImpl

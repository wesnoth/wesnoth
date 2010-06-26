/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.common.notify.Notification;

import org.eclipse.emf.ecore.EClass;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.wesnoth.wML.SimpleTag;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Simple Tag</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.SimpleTagImpl#isEndTag <em>End Tag</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class SimpleTagImpl extends RootTagImpl implements SimpleTag
{
  /**
   * The default value of the '{@link #isEndTag() <em>End Tag</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #isEndTag()
   * @generated
   * @ordered
   */
  protected static final boolean END_TAG_EDEFAULT = false;

  /**
   * The cached value of the '{@link #isEndTag() <em>End Tag</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #isEndTag()
   * @generated
   * @ordered
   */
  protected boolean endTag = END_TAG_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected SimpleTagImpl()
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
    return WMLPackage.Literals.SIMPLE_TAG;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean isEndTag()
  {
    return endTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setEndTag(boolean newEndTag)
  {
    boolean oldEndTag = endTag;
    endTag = newEndTag;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.SIMPLE_TAG__END_TAG, oldEndTag, endTag));
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
      case WMLPackage.SIMPLE_TAG__END_TAG:
        return isEndTag();
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
      case WMLPackage.SIMPLE_TAG__END_TAG:
        setEndTag((Boolean)newValue);
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
      case WMLPackage.SIMPLE_TAG__END_TAG:
        setEndTag(END_TAG_EDEFAULT);
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
      case WMLPackage.SIMPLE_TAG__END_TAG:
        return endTag != END_TAG_EDEFAULT;
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
    result.append(" (endTag: ");
    result.append(endTag);
    result.append(')');
    return result.toString();
  }

} //SimpleTagImpl

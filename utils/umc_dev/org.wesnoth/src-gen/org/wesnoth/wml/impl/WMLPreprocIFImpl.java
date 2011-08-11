/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLValuedExpression;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>WML Preproc IF</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wml.impl.WMLPreprocIFImpl#getExpressions <em>Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLPreprocIFImpl#getElses <em>Elses</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLPreprocIFImpl#getElseExpressions <em>Else Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLPreprocIFImpl#getEndName <em>End Name</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLPreprocIFImpl extends WMLRootExpressionImpl implements WMLPreprocIF
{
  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private static final long serialVersionUID = 1L;

  /**
   * The cached value of the '{@link #getExpressions() <em>Expressions</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getExpressions()
   * @generated
   * @ordered
   */
  protected EList<WMLValuedExpression> expressions;

  /**
   * The default value of the '{@link #getElses() <em>Elses</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getElses()
   * @generated
   * @ordered
   */
  protected static final String ELSES_EDEFAULT = "";

  /**
   * The cached value of the '{@link #getElses() <em>Elses</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getElses()
   * @generated
   * @ordered
   */
  protected String elses = ELSES_EDEFAULT;

  /**
   * The cached value of the '{@link #getElseExpressions() <em>Else Expressions</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getElseExpressions()
   * @generated
   * @ordered
   */
  protected EList<WMLValuedExpression> elseExpressions;

  /**
   * The default value of the '{@link #getEndName() <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEndName()
   * @generated
   * @ordered
   */
  protected static final String END_NAME_EDEFAULT = "";

  /**
   * The cached value of the '{@link #getEndName() <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEndName()
   * @generated
   * @ordered
   */
  protected String endName = END_NAME_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLPreprocIFImpl()
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
    return WmlPackage.Literals.WML_PREPROC_IF;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLValuedExpression> getExpressions()
  {
    if (expressions == null)
    {
      expressions = new EObjectContainmentEList<WMLValuedExpression>(WMLValuedExpression.class, this, WmlPackage.WML_PREPROC_IF__EXPRESSIONS);
    }
    return expressions;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getElses()
  {
    return elses;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setElses(String newElses)
  {
    String oldElses = elses;
    elses = newElses;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_PREPROC_IF__ELSES, oldElses, elses));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLValuedExpression> getElseExpressions()
  {
    if (elseExpressions == null)
    {
      elseExpressions = new EObjectContainmentEList<WMLValuedExpression>(WMLValuedExpression.class, this, WmlPackage.WML_PREPROC_IF__ELSE_EXPRESSIONS);
    }
    return elseExpressions;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getEndName()
  {
    return endName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setEndName(String newEndName)
  {
    String oldEndName = endName;
    endName = newEndName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_PREPROC_IF__END_NAME, oldEndName, endName));
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
      case WmlPackage.WML_PREPROC_IF__EXPRESSIONS:
        return ((InternalEList<?>)getExpressions()).basicRemove(otherEnd, msgs);
      case WmlPackage.WML_PREPROC_IF__ELSE_EXPRESSIONS:
        return ((InternalEList<?>)getElseExpressions()).basicRemove(otherEnd, msgs);
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
      case WmlPackage.WML_PREPROC_IF__EXPRESSIONS:
        return getExpressions();
      case WmlPackage.WML_PREPROC_IF__ELSES:
        return getElses();
      case WmlPackage.WML_PREPROC_IF__ELSE_EXPRESSIONS:
        return getElseExpressions();
      case WmlPackage.WML_PREPROC_IF__END_NAME:
        return getEndName();
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
      case WmlPackage.WML_PREPROC_IF__EXPRESSIONS:
        getExpressions().clear();
        getExpressions().addAll((Collection<? extends WMLValuedExpression>)newValue);
        return;
      case WmlPackage.WML_PREPROC_IF__ELSES:
        setElses((String)newValue);
        return;
      case WmlPackage.WML_PREPROC_IF__ELSE_EXPRESSIONS:
        getElseExpressions().clear();
        getElseExpressions().addAll((Collection<? extends WMLValuedExpression>)newValue);
        return;
      case WmlPackage.WML_PREPROC_IF__END_NAME:
        setEndName((String)newValue);
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
      case WmlPackage.WML_PREPROC_IF__EXPRESSIONS:
        getExpressions().clear();
        return;
      case WmlPackage.WML_PREPROC_IF__ELSES:
        setElses(ELSES_EDEFAULT);
        return;
      case WmlPackage.WML_PREPROC_IF__ELSE_EXPRESSIONS:
        getElseExpressions().clear();
        return;
      case WmlPackage.WML_PREPROC_IF__END_NAME:
        setEndName(END_NAME_EDEFAULT);
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
      case WmlPackage.WML_PREPROC_IF__EXPRESSIONS:
        return expressions != null && !expressions.isEmpty();
      case WmlPackage.WML_PREPROC_IF__ELSES:
        return ELSES_EDEFAULT == null ? elses != null : !ELSES_EDEFAULT.equals(elses);
      case WmlPackage.WML_PREPROC_IF__ELSE_EXPRESSIONS:
        return elseExpressions != null && !elseExpressions.isEmpty();
      case WmlPackage.WML_PREPROC_IF__END_NAME:
        return END_NAME_EDEFAULT == null ? endName != null : !END_NAME_EDEFAULT.equals(endName);
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
    result.append(" (Elses: ");
    result.append(elses);
    result.append(", endName: ");
    result.append(endName);
    result.append(')');
    return result.toString();
  }

} //WMLPreprocIFImpl

/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.util;

import org.eclipse.emf.common.notify.Adapter;
import org.eclipse.emf.common.notify.Notifier;

import org.eclipse.emf.common.notify.impl.AdapterFactoryImpl;

import org.eclipse.emf.ecore.EObject;

import org.wesnoth.wML.*;

/**
 * <!-- begin-user-doc -->
 * The <b>Adapter Factory</b> for the model.
 * It provides an adapter <code>createXXX</code> method for each class of the model.
 * <!-- end-user-doc -->
 * @see org.wesnoth.wML.WMLPackage
 * @generated
 */
public class WMLAdapterFactory extends AdapterFactoryImpl
{
  /**
   * The cached model package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected static WMLPackage modelPackage;

  /**
   * Creates an instance of the adapter factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLAdapterFactory()
  {
    if (modelPackage == null)
    {
      modelPackage = WMLPackage.eINSTANCE;
    }
  }

  /**
   * Returns whether this factory is applicable for the type of the object.
   * <!-- begin-user-doc -->
   * This implementation returns <code>true</code> if the object is either the model's package or is an instance object of the model.
   * <!-- end-user-doc -->
   * @return whether this factory is applicable for the type of the object.
   * @generated
   */
  @Override
  public boolean isFactoryForType(Object object)
  {
    if (object == modelPackage)
    {
      return true;
    }
    if (object instanceof EObject)
    {
      return ((EObject)object).eClass().getEPackage() == modelPackage;
    }
    return false;
  }

  /**
   * The switch that delegates to the <code>createXXX</code> methods.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLSwitch<Adapter> modelSwitch =
    new WMLSwitch<Adapter>()
    {
      @Override
      public Adapter caseWMLRoot(WMLRoot object)
      {
        return createWMLRootAdapter();
      }
      @Override
      public Adapter caseWMLMacro(WMLMacro object)
      {
        return createWMLMacroAdapter();
      }
      @Override
      public Adapter caseWMLTag(WMLTag object)
      {
        return createWMLTagAdapter();
      }
      @Override
      public Adapter caseWMLEndTag(WMLEndTag object)
      {
        return createWMLEndTagAdapter();
      }
      @Override
      public Adapter caseWMLKey(WMLKey object)
      {
        return createWMLKeyAdapter();
      }
      @Override
      public Adapter caseWMLKeyValue(WMLKeyValue object)
      {
        return createWMLKeyValueAdapter();
      }
      @Override
      public Adapter defaultCase(EObject object)
      {
        return createEObjectAdapter();
      }
    };

  /**
   * Creates an adapter for the <code>target</code>.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param target the object to adapt.
   * @return the adapter for the <code>target</code>.
   * @generated
   */
  @Override
  public Adapter createAdapter(Notifier target)
  {
    return modelSwitch.doSwitch((EObject)target);
  }


  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLRoot <em>Root</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLRoot
   * @generated
   */
  public Adapter createWMLRootAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLMacro <em>Macro</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLMacro
   * @generated
   */
  public Adapter createWMLMacroAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLTag <em>Tag</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLTag
   * @generated
   */
  public Adapter createWMLTagAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLEndTag <em>End Tag</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLEndTag
   * @generated
   */
  public Adapter createWMLEndTagAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLKey <em>Key</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLKey
   * @generated
   */
  public Adapter createWMLKeyAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLKeyValue <em>Key Value</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLKeyValue
   * @generated
   */
  public Adapter createWMLKeyValueAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for the default case.
   * <!-- begin-user-doc -->
   * This default implementation returns null.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @generated
   */
  public Adapter createEObjectAdapter()
  {
    return null;
  }

} //WMLAdapterFactory

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
      public Adapter caseWMLTag(WMLTag object)
      {
        return createWMLTagAdapter();
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
      public Adapter caseWMLMacroCall(WMLMacroCall object)
      {
        return createWMLMacroCallAdapter();
      }
      @Override
      public Adapter caseWMLMacroParameter(WMLMacroParameter object)
      {
        return createWMLMacroParameterAdapter();
      }
      @Override
      public Adapter caseWMLLuaCode(WMLLuaCode object)
      {
        return createWMLLuaCodeAdapter();
      }
      @Override
      public Adapter caseWMLArrayCall(WMLArrayCall object)
      {
        return createWMLArrayCallAdapter();
      }
      @Override
      public Adapter caseWMLMacroDefine(WMLMacroDefine object)
      {
        return createWMLMacroDefineAdapter();
      }
      @Override
      public Adapter caseWMLTextdomain(WMLTextdomain object)
      {
        return createWMLTextdomainAdapter();
      }
      @Override
      public Adapter caseWMLValue(WMLValue object)
      {
        return createWMLValueAdapter();
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLMacroCall <em>Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLMacroCall
   * @generated
   */
  public Adapter createWMLMacroCallAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLMacroParameter <em>Macro Parameter</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLMacroParameter
   * @generated
   */
  public Adapter createWMLMacroParameterAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLLuaCode <em>Lua Code</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLLuaCode
   * @generated
   */
  public Adapter createWMLLuaCodeAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLArrayCall <em>Array Call</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLArrayCall
   * @generated
   */
  public Adapter createWMLArrayCallAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLMacroDefine <em>Macro Define</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLMacroDefine
   * @generated
   */
  public Adapter createWMLMacroDefineAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLTextdomain <em>Textdomain</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLTextdomain
   * @generated
   */
  public Adapter createWMLTextdomainAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.WMLValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.WMLValue
   * @generated
   */
  public Adapter createWMLValueAdapter()
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

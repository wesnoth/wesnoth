/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.util;

import org.eclipse.emf.common.notify.Adapter;
import org.eclipse.emf.common.notify.Notifier;

import org.eclipse.emf.common.notify.impl.AdapterFactoryImpl;

import org.eclipse.emf.ecore.EObject;

import org.wesnoth.wml.*;

/**
 * <!-- begin-user-doc -->
 * The <b>Adapter Factory</b> for the model.
 * It provides an adapter <code>createXXX</code> method for each class of the model.
 * <!-- end-user-doc -->
 * @see org.wesnoth.wml.WmlPackage
 * @generated
 */
public class WmlAdapterFactory extends AdapterFactoryImpl
{
  /**
   * The cached model package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected static WmlPackage modelPackage;

  /**
   * Creates an instance of the adapter factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WmlAdapterFactory()
  {
    if (modelPackage == null)
    {
      modelPackage = WmlPackage.eINSTANCE;
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
  protected WmlSwitch<Adapter> modelSwitch =
    new WmlSwitch<Adapter>()
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
      public Adapter caseWMLPreprocIF(WMLPreprocIF object)
      {
        return createWMLPreprocIFAdapter();
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
      public Adapter caseMacroTokens(MacroTokens object)
      {
        return createMacroTokensAdapter();
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLRoot <em>WML Root</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLRoot
   * @generated
   */
  public Adapter createWMLRootAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLTag <em>WML Tag</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLTag
   * @generated
   */
  public Adapter createWMLTagAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLKey <em>WML Key</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLKey
   * @generated
   */
  public Adapter createWMLKeyAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLKeyValue <em>WML Key Value</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLKeyValue
   * @generated
   */
  public Adapter createWMLKeyValueAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLMacroCall <em>WML Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLMacroCall
   * @generated
   */
  public Adapter createWMLMacroCallAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLLuaCode <em>WML Lua Code</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLLuaCode
   * @generated
   */
  public Adapter createWMLLuaCodeAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLArrayCall <em>WML Array Call</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLArrayCall
   * @generated
   */
  public Adapter createWMLArrayCallAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLMacroDefine <em>WML Macro Define</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLMacroDefine
   * @generated
   */
  public Adapter createWMLMacroDefineAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLPreprocIF <em>WML Preproc IF</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLPreprocIF
   * @generated
   */
  public Adapter createWMLPreprocIFAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLTextdomain <em>WML Textdomain</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLTextdomain
   * @generated
   */
  public Adapter createWMLTextdomainAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLValue <em>WML Value</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLValue
   * @generated
   */
  public Adapter createWMLValueAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.MacroTokens <em>Macro Tokens</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.MacroTokens
   * @generated
   */
  public Adapter createMacroTokensAdapter()
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

} //WmlAdapterFactory

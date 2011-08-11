/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.util;

import java.io.Serializable;

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
      public Adapter caseWMLGrammarElement(WMLGrammarElement object)
      {
        return createWMLGrammarElementAdapter();
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
      public Adapter caseWMLMacroCallParameter(WMLMacroCallParameter object)
      {
        return createWMLMacroCallParameterAdapter();
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
      public Adapter caseWMLRootExpression(WMLRootExpression object)
      {
        return createWMLRootExpressionAdapter();
      }
      @Override
      public Adapter caseWMLExpression(WMLExpression object)
      {
        return createWMLExpressionAdapter();
      }
      @Override
      public Adapter caseWMLValuedExpression(WMLValuedExpression object)
      {
        return createWMLValuedExpressionAdapter();
      }
      @Override
      public Adapter caseWMLTextdomain(WMLTextdomain object)
      {
        return createWMLTextdomainAdapter();
      }
      @Override
      public Adapter caseWMLLuaCode(WMLLuaCode object)
      {
        return createWMLLuaCodeAdapter();
      }
      @Override
      public Adapter caseWMLMacroParameter(WMLMacroParameter object)
      {
        return createWMLMacroParameterAdapter();
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
      public Adapter caseESerializable(Serializable object)
      {
        return createESerializableAdapter();
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLGrammarElement <em>WML Grammar Element</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLGrammarElement
   * @generated
   */
  public Adapter createWMLGrammarElementAdapter()
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLMacroCallParameter <em>WML Macro Call Parameter</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLMacroCallParameter
   * @generated
   */
  public Adapter createWMLMacroCallParameterAdapter()
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLRootExpression <em>WML Root Expression</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLRootExpression
   * @generated
   */
  public Adapter createWMLRootExpressionAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLExpression <em>WML Expression</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLExpression
   * @generated
   */
  public Adapter createWMLExpressionAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLValuedExpression <em>WML Valued Expression</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLValuedExpression
   * @generated
   */
  public Adapter createWMLValuedExpressionAdapter()
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wml.WMLMacroParameter <em>WML Macro Parameter</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wml.WMLMacroParameter
   * @generated
   */
  public Adapter createWMLMacroParameterAdapter()
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
   * Creates a new adapter for an object of class '{@link java.io.Serializable <em>ESerializable</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see java.io.Serializable
   * @generated
   */
  public Adapter createESerializableAdapter()
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

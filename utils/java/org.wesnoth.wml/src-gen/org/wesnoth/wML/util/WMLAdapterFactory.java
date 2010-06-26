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
      public Adapter caseRoot(Root object)
      {
        return createRootAdapter();
      }
      @Override
      public Adapter caseTextDomain(TextDomain object)
      {
        return createTextDomainAdapter();
      }
      @Override
      public Adapter casePreprocessor(Preprocessor object)
      {
        return createPreprocessorAdapter();
      }
      @Override
      public Adapter caseMacro(Macro object)
      {
        return createMacroAdapter();
      }
      @Override
      public Adapter casePathInclude(PathInclude object)
      {
        return createPathIncludeAdapter();
      }
      @Override
      public Adapter caseRootType(RootType object)
      {
        return createRootTypeAdapter();
      }
      @Override
      public Adapter caseRootTag(RootTag object)
      {
        return createRootTagAdapter();
      }
      @Override
      public Adapter caseSimpleTag(SimpleTag object)
      {
        return createSimpleTagAdapter();
      }
      @Override
      public Adapter caseAddedTag(AddedTag object)
      {
        return createAddedTagAdapter();
      }
      @Override
      public Adapter caseAttributes(Attributes object)
      {
        return createAttributesAdapter();
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
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.Root <em>Root</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.Root
   * @generated
   */
  public Adapter createRootAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.TextDomain <em>Text Domain</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.TextDomain
   * @generated
   */
  public Adapter createTextDomainAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.Preprocessor <em>Preprocessor</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.Preprocessor
   * @generated
   */
  public Adapter createPreprocessorAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.Macro <em>Macro</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.Macro
   * @generated
   */
  public Adapter createMacroAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.PathInclude <em>Path Include</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.PathInclude
   * @generated
   */
  public Adapter createPathIncludeAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.RootType <em>Root Type</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.RootType
   * @generated
   */
  public Adapter createRootTypeAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.RootTag <em>Root Tag</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.RootTag
   * @generated
   */
  public Adapter createRootTagAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.SimpleTag <em>Simple Tag</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.SimpleTag
   * @generated
   */
  public Adapter createSimpleTagAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.AddedTag <em>Added Tag</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.AddedTag
   * @generated
   */
  public Adapter createAddedTagAdapter()
  {
    return null;
  }

  /**
   * Creates a new adapter for an object of class '{@link org.wesnoth.wML.Attributes <em>Attributes</em>}'.
   * <!-- begin-user-doc -->
   * This default implementation returns null so that we can easily ignore cases;
   * it's useful to ignore a case when inheritance will catch all the cases anyway.
   * <!-- end-user-doc -->
   * @return the new adapter.
   * @see org.wesnoth.wML.Attributes
   * @generated
   */
  public Adapter createAttributesAdapter()
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

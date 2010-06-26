/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML.util;

import java.util.List;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;

import org.wesnoth.wML.*;

/**
 * <!-- begin-user-doc -->
 * The <b>Switch</b> for the model's inheritance hierarchy.
 * It supports the call {@link #doSwitch(EObject) doSwitch(object)}
 * to invoke the <code>caseXXX</code> method for each class of the model,
 * starting with the actual class of the object
 * and proceeding up the inheritance hierarchy
 * until a non-null result is returned,
 * which is the result of the switch.
 * <!-- end-user-doc -->
 * @see org.wesnoth.wML.WMLPackage
 * @generated
 */
public class WMLSwitch<T>
{
  /**
   * The cached model package
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected static WMLPackage modelPackage;

  /**
   * Creates an instance of the switch.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLSwitch()
  {
    if (modelPackage == null)
    {
      modelPackage = WMLPackage.eINSTANCE;
    }
  }

  /**
   * Calls <code>caseXXX</code> for each class of the model until one returns a non null result; it yields that result.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the first non-null result returned by a <code>caseXXX</code> call.
   * @generated
   */
  public T doSwitch(EObject theEObject)
  {
    return doSwitch(theEObject.eClass(), theEObject);
  }

  /**
   * Calls <code>caseXXX</code> for each class of the model until one returns a non null result; it yields that result.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the first non-null result returned by a <code>caseXXX</code> call.
   * @generated
   */
  protected T doSwitch(EClass theEClass, EObject theEObject)
  {
    if (theEClass.eContainer() == modelPackage)
    {
      return doSwitch(theEClass.getClassifierID(), theEObject);
    }
    else
    {
      List<EClass> eSuperTypes = theEClass.getESuperTypes();
      return
        eSuperTypes.isEmpty() ?
          defaultCase(theEObject) :
          doSwitch(eSuperTypes.get(0), theEObject);
    }
  }

  /**
   * Calls <code>caseXXX</code> for each class of the model until one returns a non null result; it yields that result.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the first non-null result returned by a <code>caseXXX</code> call.
   * @generated
   */
  protected T doSwitch(int classifierID, EObject theEObject)
  {
    switch (classifierID)
    {
      case WMLPackage.ROOT:
      {
        Root root = (Root)theEObject;
        T result = caseRoot(root);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.TEXT_DOMAIN:
      {
        TextDomain textDomain = (TextDomain)theEObject;
        T result = caseTextDomain(textDomain);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.PREPROCESSOR:
      {
        Preprocessor preprocessor = (Preprocessor)theEObject;
        T result = casePreprocessor(preprocessor);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.MACRO:
      {
        Macro macro = (Macro)theEObject;
        T result = caseMacro(macro);
        if (result == null) result = casePreprocessor(macro);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.PATH_INCLUDE:
      {
        PathInclude pathInclude = (PathInclude)theEObject;
        T result = casePathInclude(pathInclude);
        if (result == null) result = casePreprocessor(pathInclude);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.ROOT_TYPE:
      {
        RootType rootType = (RootType)theEObject;
        T result = caseRootType(rootType);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.ROOT_TAG:
      {
        RootTag rootTag = (RootTag)theEObject;
        T result = caseRootTag(rootTag);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.SIMPLE_TAG:
      {
        SimpleTag simpleTag = (SimpleTag)theEObject;
        T result = caseSimpleTag(simpleTag);
        if (result == null) result = caseRootTag(simpleTag);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.ADDED_TAG:
      {
        AddedTag addedTag = (AddedTag)theEObject;
        T result = caseAddedTag(addedTag);
        if (result == null) result = caseRootTag(addedTag);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      case WMLPackage.ATTRIBUTES:
      {
        Attributes attributes = (Attributes)theEObject;
        T result = caseAttributes(attributes);
        if (result == null) result = defaultCase(theEObject);
        return result;
      }
      default: return defaultCase(theEObject);
    }
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Root</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Root</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseRoot(Root object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Text Domain</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Text Domain</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseTextDomain(TextDomain object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Preprocessor</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Preprocessor</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T casePreprocessor(Preprocessor object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Macro</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Macro</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseMacro(Macro object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Path Include</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Path Include</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T casePathInclude(PathInclude object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Root Type</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Root Type</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseRootType(RootType object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Root Tag</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Root Tag</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseRootTag(RootTag object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Simple Tag</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Simple Tag</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseSimpleTag(SimpleTag object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Added Tag</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Added Tag</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseAddedTag(AddedTag object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>Attributes</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>Attributes</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
   * @generated
   */
  public T caseAttributes(Attributes object)
  {
    return null;
  }

  /**
   * Returns the result of interpreting the object as an instance of '<em>EObject</em>'.
   * <!-- begin-user-doc -->
   * This implementation returns null;
   * returning a non-null result will terminate the switch, but this is the last case anyway.
   * <!-- end-user-doc -->
   * @param object the target of the switch.
   * @return the result of interpreting the object as an instance of '<em>EObject</em>'.
   * @see #doSwitch(org.eclipse.emf.ecore.EObject)
   * @generated
   */
  public T defaultCase(EObject object)
  {
    return null;
  }

} //WMLSwitch

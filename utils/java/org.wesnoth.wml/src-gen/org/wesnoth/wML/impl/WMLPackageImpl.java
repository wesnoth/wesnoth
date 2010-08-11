/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EPackage;
import org.eclipse.emf.ecore.EReference;

import org.eclipse.emf.ecore.impl.EPackageImpl;

import org.wesnoth.wML.WMLAbstractKeyValue;
import org.wesnoth.wML.WMLAbstractMacroCall;
import org.wesnoth.wML.WMLFactory;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLMacroInclude;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model <b>Package</b>.
 * <!-- end-user-doc -->
 * @generated
 */
public class WMLPackageImpl extends EPackageImpl implements WMLPackage
{
  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlRootEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlTagEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlKeyEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlAbstractMacroCallEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroIncludeEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroCallEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlAbstractKeyValueEClass = null;

  /**
   * Creates an instance of the model <b>Package</b>, registered with
   * {@link org.eclipse.emf.ecore.EPackage.Registry EPackage.Registry} by the package
   * package URI value.
   * <p>Note: the correct way to create the package is via the static
   * factory method {@link #init init()}, which also performs
   * initialization of the package, or returns the registered package,
   * if one already exists.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.eclipse.emf.ecore.EPackage.Registry
   * @see org.wesnoth.wML.WMLPackage#eNS_URI
   * @see #init()
   * @generated
   */
  private WMLPackageImpl()
  {
    super(eNS_URI, WMLFactory.eINSTANCE);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private static boolean isInited = false;

  /**
   * Creates, registers, and initializes the <b>Package</b> for this model, and for any others upon which it depends.
   * 
   * <p>This method is used to initialize {@link WMLPackage#eINSTANCE} when that field is accessed.
   * Clients should not invoke it directly. Instead, they should simply access that field to obtain the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #eNS_URI
   * @see #createPackageContents()
   * @see #initializePackageContents()
   * @generated
   */
  public static WMLPackage init()
  {
    if (isInited) return (WMLPackage)EPackage.Registry.INSTANCE.getEPackage(WMLPackage.eNS_URI);

    // Obtain or create and register package
    WMLPackageImpl theWMLPackage = (WMLPackageImpl)(EPackage.Registry.INSTANCE.get(eNS_URI) instanceof WMLPackageImpl ? EPackage.Registry.INSTANCE.get(eNS_URI) : new WMLPackageImpl());

    isInited = true;

    // Create package meta-data objects
    theWMLPackage.createPackageContents();

    // Initialize created meta-data
    theWMLPackage.initializePackageContents();

    // Mark meta-data to indicate it can't be changed
    theWMLPackage.freeze();

  
    // Update the registry and return the package
    EPackage.Registry.INSTANCE.put(WMLPackage.eNS_URI, theWMLPackage);
    return theWMLPackage;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLRoot()
  {
    return wmlRootEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLRoot_Tags()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLRoot_MacroCalls()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLTag()
  {
    return wmlTagEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_Plus()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_Name()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Tags()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Keys()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_MacroCalls()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_EndName()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(5);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLKey()
  {
    return wmlKeyEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKey_Name()
  {
    return (EAttribute)wmlKeyEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLKey_Value()
  {
    return (EReference)wmlKeyEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLKey_ExtraArgs()
  {
    return (EReference)wmlKeyEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLAbstractMacroCall()
  {
    return wmlAbstractMacroCallEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacroInclude()
  {
    return wmlMacroIncludeEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroInclude_Path()
  {
    return (EAttribute)wmlMacroIncludeEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacroCall()
  {
    return wmlMacroCallEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroCall_Name()
  {
    return (EAttribute)wmlMacroCallEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroCall_Params()
  {
    return (EReference)wmlMacroCallEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLAbstractKeyValue()
  {
    return wmlAbstractKeyValueEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLFactory getWMLFactory()
  {
    return (WMLFactory)getEFactoryInstance();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private boolean isCreated = false;

  /**
   * Creates the meta-model objects for the package.  This method is
   * guarded to have no affect on any invocation but its first.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void createPackageContents()
  {
    if (isCreated) return;
    isCreated = true;

    // Create classes and their features
    wmlRootEClass = createEClass(WML_ROOT);
    createEReference(wmlRootEClass, WML_ROOT__TAGS);
    createEReference(wmlRootEClass, WML_ROOT__MACRO_CALLS);

    wmlTagEClass = createEClass(WML_TAG);
    createEAttribute(wmlTagEClass, WML_TAG__PLUS);
    createEAttribute(wmlTagEClass, WML_TAG__NAME);
    createEReference(wmlTagEClass, WML_TAG__TAGS);
    createEReference(wmlTagEClass, WML_TAG__KEYS);
    createEReference(wmlTagEClass, WML_TAG__MACRO_CALLS);
    createEAttribute(wmlTagEClass, WML_TAG__END_NAME);

    wmlKeyEClass = createEClass(WML_KEY);
    createEAttribute(wmlKeyEClass, WML_KEY__NAME);
    createEReference(wmlKeyEClass, WML_KEY__VALUE);
    createEReference(wmlKeyEClass, WML_KEY__EXTRA_ARGS);

    wmlAbstractMacroCallEClass = createEClass(WML_ABSTRACT_MACRO_CALL);

    wmlMacroIncludeEClass = createEClass(WML_MACRO_INCLUDE);
    createEAttribute(wmlMacroIncludeEClass, WML_MACRO_INCLUDE__PATH);

    wmlMacroCallEClass = createEClass(WML_MACRO_CALL);
    createEAttribute(wmlMacroCallEClass, WML_MACRO_CALL__NAME);
    createEReference(wmlMacroCallEClass, WML_MACRO_CALL__PARAMS);

    wmlAbstractKeyValueEClass = createEClass(WML_ABSTRACT_KEY_VALUE);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private boolean isInitialized = false;

  /**
   * Complete the initialization of the package and its meta-model.  This
   * method is guarded to have no affect on any invocation but its first.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void initializePackageContents()
  {
    if (isInitialized) return;
    isInitialized = true;

    // Initialize package
    setName(eNAME);
    setNsPrefix(eNS_PREFIX);
    setNsURI(eNS_URI);

    // Create type parameters

    // Set bounds for type parameters

    // Add supertypes to classes
    wmlMacroIncludeEClass.getESuperTypes().add(this.getWMLAbstractMacroCall());
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLAbstractMacroCall());
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLAbstractKeyValue());

    // Initialize classes and features; add operations and parameters
    initEClass(wmlRootEClass, WMLRoot.class, "WMLRoot", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLRoot_Tags(), this.getWMLTag(), null, "tags", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLRoot_MacroCalls(), this.getWMLAbstractMacroCall(), null, "macroCalls", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlTagEClass, WMLTag.class, "WMLTag", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLTag_Plus(), ecorePackage.getEBoolean(), "plus", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Tags(), this.getWMLTag(), null, "tags", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Keys(), this.getWMLKey(), null, "keys", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_MacroCalls(), this.getWMLAbstractMacroCall(), null, "macroCalls", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag_EndName(), ecorePackage.getEString(), "endName", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlKeyEClass, WMLKey.class, "WMLKey", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLKey_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLKey_Value(), this.getWMLAbstractKeyValue(), null, "value", null, 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLKey_ExtraArgs(), this.getWMLAbstractKeyValue(), null, "extraArgs", null, 0, -1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlAbstractMacroCallEClass, WMLAbstractMacroCall.class, "WMLAbstractMacroCall", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlMacroIncludeEClass, WMLMacroInclude.class, "WMLMacroInclude", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacroInclude_Path(), ecorePackage.getEString(), "path", null, 0, 1, WMLMacroInclude.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroCallEClass, WMLMacroCall.class, "WMLMacroCall", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacroCall_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroCall_Params(), this.getWMLAbstractKeyValue(), null, "params", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlAbstractKeyValueEClass, WMLAbstractKeyValue.class, "WMLAbstractKeyValue", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    // Create resource
    createResource(eNS_URI);
  }

} //WMLPackageImpl

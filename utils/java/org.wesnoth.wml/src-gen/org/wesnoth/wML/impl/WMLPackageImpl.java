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

import org.wesnoth.wML.WMLAbstractMacroCall;
import org.wesnoth.wML.WMLFactory;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLKeyExtraArgs;
import org.wesnoth.wML.WMLKeyValue;
import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLMacroDefine;
import org.wesnoth.wML.WMLMacroInclude;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;
import org.wesnoth.wML.WMLTextdomain;

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
  private EClass wmlMacroDefineEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlTextdomainEClass = null;

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
  private EClass wmlKeyExtraArgsEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlKeyValueEClass = null;

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
  public EReference getWMLRoot_Macros()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLRoot_MacrosDefines()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(2);
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
  public EReference getWMLTag_Macros()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_MacrosDefines()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Keys()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(5);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_EndName()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(6);
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
  public EAttribute getWMLAbstractMacroCall_Name()
  {
    return (EAttribute)wmlAbstractMacroCallEClass.getEStructuralFeatures().get(0);
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
  public EClass getWMLMacroCall()
  {
    return wmlMacroCallEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroCall_Args()
  {
    return (EAttribute)wmlMacroCallEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroCall_Params()
  {
    return (EAttribute)wmlMacroCallEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroCall_Tags()
  {
    return (EReference)wmlMacroCallEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroCall_Macros()
  {
    return (EReference)wmlMacroCallEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroCall_MacrosDefines()
  {
    return (EReference)wmlMacroCallEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroCall_Keys()
  {
    return (EReference)wmlMacroCallEClass.getEStructuralFeatures().get(5);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacroDefine()
  {
    return wmlMacroDefineEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroDefine_Params()
  {
    return (EAttribute)wmlMacroDefineEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroDefine_Tags()
  {
    return (EReference)wmlMacroDefineEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroDefine_Macros()
  {
    return (EReference)wmlMacroDefineEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroDefine_MacrosDefines()
  {
    return (EReference)wmlMacroDefineEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroDefine_Keys()
  {
    return (EReference)wmlMacroDefineEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLTextdomain()
  {
    return wmlTextdomainEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTextdomain_Name()
  {
    return (EAttribute)wmlTextdomainEClass.getEStructuralFeatures().get(0);
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
  public EClass getWMLKeyExtraArgs()
  {
    return wmlKeyExtraArgsEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLKeyValue()
  {
    return wmlKeyValueEClass;
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
    createEReference(wmlRootEClass, WML_ROOT__MACROS);
    createEReference(wmlRootEClass, WML_ROOT__MACROS_DEFINES);

    wmlTagEClass = createEClass(WML_TAG);
    createEAttribute(wmlTagEClass, WML_TAG__PLUS);
    createEAttribute(wmlTagEClass, WML_TAG__NAME);
    createEReference(wmlTagEClass, WML_TAG__TAGS);
    createEReference(wmlTagEClass, WML_TAG__MACROS);
    createEReference(wmlTagEClass, WML_TAG__MACROS_DEFINES);
    createEReference(wmlTagEClass, WML_TAG__KEYS);
    createEAttribute(wmlTagEClass, WML_TAG__END_NAME);

    wmlAbstractMacroCallEClass = createEClass(WML_ABSTRACT_MACRO_CALL);
    createEAttribute(wmlAbstractMacroCallEClass, WML_ABSTRACT_MACRO_CALL__NAME);

    wmlMacroIncludeEClass = createEClass(WML_MACRO_INCLUDE);

    wmlMacroCallEClass = createEClass(WML_MACRO_CALL);
    createEAttribute(wmlMacroCallEClass, WML_MACRO_CALL__ARGS);
    createEAttribute(wmlMacroCallEClass, WML_MACRO_CALL__PARAMS);
    createEReference(wmlMacroCallEClass, WML_MACRO_CALL__TAGS);
    createEReference(wmlMacroCallEClass, WML_MACRO_CALL__MACROS);
    createEReference(wmlMacroCallEClass, WML_MACRO_CALL__MACROS_DEFINES);
    createEReference(wmlMacroCallEClass, WML_MACRO_CALL__KEYS);

    wmlMacroDefineEClass = createEClass(WML_MACRO_DEFINE);
    createEAttribute(wmlMacroDefineEClass, WML_MACRO_DEFINE__PARAMS);
    createEReference(wmlMacroDefineEClass, WML_MACRO_DEFINE__TAGS);
    createEReference(wmlMacroDefineEClass, WML_MACRO_DEFINE__MACROS);
    createEReference(wmlMacroDefineEClass, WML_MACRO_DEFINE__MACROS_DEFINES);
    createEReference(wmlMacroDefineEClass, WML_MACRO_DEFINE__KEYS);

    wmlTextdomainEClass = createEClass(WML_TEXTDOMAIN);
    createEAttribute(wmlTextdomainEClass, WML_TEXTDOMAIN__NAME);

    wmlKeyEClass = createEClass(WML_KEY);
    createEAttribute(wmlKeyEClass, WML_KEY__NAME);
    createEReference(wmlKeyEClass, WML_KEY__VALUE);
    createEReference(wmlKeyEClass, WML_KEY__EXTRA_ARGS);

    wmlKeyExtraArgsEClass = createEClass(WML_KEY_EXTRA_ARGS);

    wmlKeyValueEClass = createEClass(WML_KEY_VALUE);
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
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLKeyExtraArgs());
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLKeyValue());

    // Initialize classes and features; add operations and parameters
    initEClass(wmlRootEClass, WMLRoot.class, "WMLRoot", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLRoot_Tags(), this.getWMLTag(), null, "tags", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLRoot_Macros(), this.getWMLAbstractMacroCall(), null, "macros", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLRoot_MacrosDefines(), this.getWMLMacroDefine(), null, "macrosDefines", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlTagEClass, WMLTag.class, "WMLTag", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLTag_Plus(), ecorePackage.getEBoolean(), "plus", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Tags(), this.getWMLTag(), null, "tags", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Macros(), this.getWMLAbstractMacroCall(), null, "macros", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_MacrosDefines(), this.getWMLMacroDefine(), null, "macrosDefines", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Keys(), this.getWMLKey(), null, "keys", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag_EndName(), ecorePackage.getEString(), "endName", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlAbstractMacroCallEClass, WMLAbstractMacroCall.class, "WMLAbstractMacroCall", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLAbstractMacroCall_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLAbstractMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroIncludeEClass, WMLMacroInclude.class, "WMLMacroInclude", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlMacroCallEClass, WMLMacroCall.class, "WMLMacroCall", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacroCall_Args(), ecorePackage.getEString(), "args", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, !IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLMacroCall_Params(), ecorePackage.getEString(), "params", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, !IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroCall_Tags(), this.getWMLTag(), null, "tags", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroCall_Macros(), this.getWMLMacroCall(), null, "macros", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroCall_MacrosDefines(), this.getWMLMacroDefine(), null, "macrosDefines", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroCall_Keys(), this.getWMLKey(), null, "keys", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroDefineEClass, WMLMacroDefine.class, "WMLMacroDefine", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacroDefine_Params(), ecorePackage.getEString(), "params", null, 0, -1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, !IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroDefine_Tags(), this.getWMLTag(), null, "tags", null, 0, -1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroDefine_Macros(), this.getWMLMacroCall(), null, "macros", null, 0, -1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroDefine_MacrosDefines(), this.getWMLMacroDefine(), null, "macrosDefines", null, 0, -1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroDefine_Keys(), this.getWMLKey(), null, "keys", null, 0, -1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlTextdomainEClass, WMLTextdomain.class, "WMLTextdomain", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLTextdomain_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLTextdomain.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlKeyEClass, WMLKey.class, "WMLKey", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLKey_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLKey_Value(), this.getWMLKeyValue(), null, "value", null, 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLKey_ExtraArgs(), this.getWMLKeyExtraArgs(), null, "extraArgs", null, 0, -1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlKeyExtraArgsEClass, WMLKeyExtraArgs.class, "WMLKeyExtraArgs", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlKeyValueEClass, WMLKeyValue.class, "WMLKeyValue", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    // Create resource
    createResource(eNS_URI);
  }

} //WMLPackageImpl
